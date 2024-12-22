#include "TestDirector.hh"

#include <algorithm>
#include <cstdlib>
#include <game/kart/KartObjectManager.hh>
#include <game/system/RaceManager.hh>

#include <abstract/File.hh>
#include <host/System.hh>

#include <format>
#include <print>
#include <ranges>

namespace Test {

// We use an unscoped enum to avoid static_casting in all usecases
// This is defined in the source due to its lack of scoping
enum Changelog {
    Initial = 1,
    AddedExtVel = 2,
    AddedIntVel = 3,
    AddedSpeed = 4,
    AddedRotation = 5,
    AddedCheckpoints = 6,
};

TestDirector::TestDirector(const std::span<u8> &suiteData) {
    Abstract::File::Remove("results.txt");
    EGG::RamStream stream = EGG::RamStream(suiteData.data(), suiteData.size());
    stream.setEndian(std::endian::big);
    parseSuite(stream);
    init();
}

TestDirector::~TestDirector() = default;

void TestDirector::parseSuite(EGG::RamStream &stream) {
    constexpr u32 TEST_HEADER_SIGNATURE = 0x54535448; // TSTH
    constexpr u32 TEST_FOOTER_SIGNATURE = 0x54535446; // TSTF
    constexpr u16 SUITE_MAJOR_VER = 1;
    constexpr u16 SUITE_MAX_MINOR_VER = 0;

    u16 numTestCases = stream.read_u16();
    u16 testMajorVer = stream.read_u16();
    u16 testMinorVer = stream.read_u16();

    if (testMajorVer != SUITE_MAJOR_VER || testMinorVer > SUITE_MAX_MINOR_VER) {
        PANIC("Version not supported! Provided file is %d.%d while Kinoko supports up to %d.%d",
                testMajorVer, testMinorVer, SUITE_MAJOR_VER, SUITE_MAX_MINOR_VER);
    }

    for (u16 i = 0; i < numTestCases; ++i) {
        // Validate alignment
        if (stream.read_u32() != TEST_HEADER_SIGNATURE) {
            PANIC("Invalid binary data for test case!");
        }

        u16 totalSize = stream.read_u16();
        TestCase testCase;

        u16 nameLen = stream.read_u16();
        testCase.name = stream.read_string();
        if (nameLen != testCase.name.size() + 1) {
            PANIC("Test case name length mismatch!");
        }

        u16 rkgPathLen = stream.read_u16();
        testCase.rkgPath = stream.read_string();
        if (rkgPathLen != testCase.rkgPath.size() + 1) {
            PANIC("Test case RKG Path length mismatch!");
        }

        u16 krkgPathLen = stream.read_u16();
        testCase.krkgPath = stream.read_string();
        if (krkgPathLen != testCase.krkgPath.size() + 1) {
            PANIC("Test case KRKG Path length mismatch!");
        }

        testCase.targetFrame = stream.read_u16();

        // Validate alignment
        if (stream.read_u32() != TEST_FOOTER_SIGNATURE) {
            PANIC("Invalid binary data for test case!");
        }

        if (totalSize != sizeof(u16) * 4 + nameLen + rkgPathLen + krkgPathLen) {
            PANIC("Unexpected bytes in test case");
        }

        m_tests.emplace_back(testCase, i);
    }
}

void TestDirector::init() {
    for (auto &test: m_tests) {
        size_t size;
        u8 *krkg = Abstract::File::Load(test.testCase.krkgPath.data(), size);
        test.stream = EGG::RamStream(krkg, static_cast<u32>(size));
        m_currentFrame = -1;
        test.sync = true;

        // Initialize endianness for the RAM stream
        u16 mark = *reinterpret_cast<u16 *>(krkg + offsetof(TestHeader, byteOrderMark));
        std::endian endian = parse<u16>(mark) == 0xfeff ? std::endian::big : std::endian::little;
        test.stream.setEndian(endian);

        readHeader(test);

        ASSERT(test.stream.read_u32() == test.stream.index());
    }
}

bool TestDirector::calc() {
    for (auto &test : std::ranges::views::filter(m_tests, [] (auto & t) {return !t.done;})) {
        // Check if we're out of frames
        u16 targetFrame = test.testCase.targetFrame;
        ASSERT(targetFrame <= test.frameCount);
        if (++m_currentFrame > targetFrame) {
            REPORT("Test Case Passed: %s [%d / %zu]", test.testCase.name.c_str(), targetFrame, test.frameCount);
            test.done = true;
        }

        // Test the current frame
        TestData data = findNextEntry(test.stream);
        this->test(data, test);
    }

    if (std::all_of(
        m_tests.begin(),
        m_tests.end(),
        [] (auto &t) { return t.done; }
    )) {
        return false;
    }
    return true;
}

void TestDirector::test(const TestData &data, RunningTest test) {
    auto *object = Kart::KartObjectManager::Instance()->object(test.idx);
    const auto &pos = object->pos();
    const auto &fullRot = object->fullRot();
    const auto &extVel = object->extVel();
    const auto &intVel = object->intVel();
    f32 speed = object->speed();
    f32 acceleration = object->acceleration();
    f32 softSpeedLimit = object->softSpeedLimit();
    const auto &mainRot = object->mainRot();
    const auto &angVel2 = object->angVel2();

    const auto &player = System::RaceManager::Instance()->player(test.idx);
    f32 raceCompletion = player.raceCompletion();
    u16 checkpointId = player.checkpointId();
    u8 jugemId = player.jugemId();

    switch (m_versionMinor) {
    case Changelog::AddedCheckpoints:
        checkDesync(test, data.raceCompletion, raceCompletion, "raceCompletion");
        checkDesync(test, data.checkpointId, checkpointId, "checkpointId");
        checkDesync(test, data.jugemId, jugemId, "jugemId");
        [[fallthrough]];
    case Changelog::AddedRotation:
        checkDesync(test, data.mainRot, mainRot, "mainRot");
        checkDesync(test, data.angVel2, angVel2, "angVel2");
        [[fallthrough]];
    case Changelog::AddedSpeed:
        checkDesync(test, data.speed, speed, "speed");
        checkDesync(test, data.acceleration, acceleration, "acceleration");
        checkDesync(test, data.softSpeedLimit, softSpeedLimit, "softSpeedLimit");
        [[fallthrough]];
    case Changelog::AddedIntVel:
        checkDesync(test, data.intVel, intVel, "intVel");
        [[fallthrough]];
    case Changelog::AddedExtVel:
        checkDesync(test, data.extVel, extVel, "extVel");
        [[fallthrough]];
    default:
        checkDesync(test, data.pos, pos, "pos");
        checkDesync(test, data.fullRot, fullRot, "fullRot");
    }
}

void TestDirector::writeTestOutput() const {
    // std::string outStr(testCase().name.data());
    // outStr += "\n" + std::string(m_sync ? "1" : "0") + "\n";
    // outStr += std::to_string(testCase().targetFrame) + "\n";
    // outStr += std::to_string(m_frameCount) + "\n";
    // Abstract::File::Append("results.txt", outStr.c_str(), outStr.size());
}

// Pops the first test case in the queue.
// Also free the KRKG buffer for that test case.
// Returns whether or not there are remaining test cases.
bool TestDirector::popTestCase() {
    // ASSERT(m_tests.size() > 0);
    std::println("poppi");
    exit(42);
    // m_testCases.pop();
    // delete[] m_streams.data();

    // return !m_testCases.empty();
}

TestData TestDirector::findNextEntry(EGG::RamStream &stream) {
    EGG::Vector3f pos;
    EGG::Quatf fullRot;
    EGG::Vector3f extVel;
    EGG::Vector3f intVel;
    f32 speed = 0.0f;
    f32 acceleration = 0.0f;
    f32 softSpeedLimit = 0.0f;
    EGG::Quatf mainRot;
    EGG::Vector3f angVel2;
    f32 raceCompletion = 0.0f;
    u16 checkpointId = 0;
    u8 jugemId = 0;

    pos.read(stream);
    fullRot.read(stream);

    if (m_versionMinor >= Changelog::AddedExtVel) {
        extVel.read(stream);
    }

    if (m_versionMinor >= Changelog::AddedIntVel) {
        intVel.read(stream);
    }

    if (m_versionMinor >= Changelog::AddedSpeed) {
        speed = stream.read_f32();
        acceleration = stream.read_f32();
        softSpeedLimit = stream.read_f32();
    }

    if (m_versionMinor >= Changelog::AddedRotation) {
        mainRot.read(stream);
        angVel2.read(stream);
    }

    if (m_versionMinor >= Changelog::AddedCheckpoints) {
        raceCompletion = stream.read_f32();
        checkpointId = stream.read_u16();
        jugemId = stream.read_u8();
        stream.skip(1);
    }

    TestData data;
    data.pos = pos;
    data.fullRot = fullRot;
    data.extVel = extVel;
    data.intVel = intVel;
    data.speed = speed;
    data.acceleration = acceleration;
    data.softSpeedLimit = softSpeedLimit;
    data.mainRot = mainRot;
    data.angVel2 = angVel2;
    data.raceCompletion = raceCompletion;
    data.checkpointId = checkpointId;
    data.jugemId = jugemId;
    return data;
}

// const TestCase &TestDirector::testCase() const {
//     ASSERT(m_testCases.size() > 0);
//     return m_testCases.front();
// }

// bool TestDirector::sync() const {
//     return m_sync;
// }

void TestDirector::OnInit(System::RaceConfig *config, void * /* arg */) {
    const auto *testDirector = Host::KSystem::Instance().testDirector();
    for (auto &test: testDirector->m_tests) {
        size_t size;
        u8 *rkg = Abstract::File::Load(test.testCase.rkgPath.data(), size);
        config->setGhost(rkg, test.idx);
        delete[] rkg;
    }

    auto &players = config->raceScenario().players;
    players.emplace_back();
    players.emplace_back();
    players.front().type = System::RaceConfig::Player::Type::Ghost;
    players.back().type = System::RaceConfig::Player::Type::Ghost;
}

void TestDirector::readHeader(RunningTest & test) {
    constexpr u32 KRKG_SIGNATURE = 0x4b524b47; // KRKG

    ASSERT(test.stream.read_u32() == KRKG_SIGNATURE);
    test.stream.skip(2);
    test.frameCount = test.stream.read_u16();
    m_versionMajor = test.stream.read_u16();
    m_versionMinor = test.stream.read_u16();
}

} // namespace Test
