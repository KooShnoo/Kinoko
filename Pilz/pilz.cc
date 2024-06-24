#include "Common.hh"
#include "game/system/KPadController.hh"
#include <cassert>
#include <cstddef>
#include <game/kart/KartObjectProxy.hh>
#include <game/system/RaceConfig.hh>
#include <optional>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <egg/core/SceneManager.hh>
#include <egg/math/Vector.hh>
#include <game/kart/KartObjectManager.hh>
#include <game/system/KPadDirector.hh>
#include <game/system/RaceManager.hh>
#include <host/SceneCreatorDynamic.hh>

void JSlog(const char* msg);

static EGG::SceneManager *sceneMgr;
constexpr const auto defaultRaceSelections = System::RaceConfig::RaceSelections{
    .course = Course::N64_DKs_Jungle_Parkway,
    .character = Character::Mario,
    .vehicle = Vehicle::Mach_Bike,
    .type = System::RaceConfig::Player::Type::Local,
};

class PilzController: public System::KPadController {
public:
    System::ControlSource controlSource() override { return System::ControlSource::Classic; }

    void reset(bool driftIsAuto) override {
        m_driftIsAuto = driftIsAuto;
        m_raceInputState.reset();
        m_connected = true;
    }

    void setButtons(u16 newButtons, f32 stickX, f32 stickY) {
        m_raceInputState.buttons = newButtons;
        m_raceInputState.stick = EGG::Vector2f(stickX, stickY);
    }
    u16 getButtons() { return m_raceInputState.buttons; }
    // u16 JSButtons;
};

struct PilzPlayerTransform {
    s32 timer;
    std::array<f32, 3> pos;
    std::array<f32, 3> rot;
};

static PilzController *sController;
class Pilz {
public:
    static void init() {
        sPlayerTransform = new PilzPlayerTransform;
        sController = new PilzController;
        sSelections = std::nullopt;
        auto *sceneCreator = new Host::SceneCreatorDynamic;
        assert(sceneCreator);
        sceneMgr = new EGG::SceneManager(sceneCreator);

        sController->setDriftIsAuto(false);
        System::RaceConfig::setSelections(defaultRaceSelections);
    }

    static void updateTransform() {
        const auto player = Kart::KartObjectManager::Instance()->object(0);
        const auto &pos = player->pos();
        const auto &mainRot = player->mainRot();
        auto rot = mainRot.rotateVector(EGG::Vector3f::unit);
        rot.normalise();
        sPlayerTransform->timer = System::RaceManager::Instance()->getTimer();
        sPlayerTransform->pos = {pos.x, pos.y, pos.z};
        sPlayerTransform->rot = {rot.x, rot.y, rot.z};
    }

    static PilzPlayerTransform *sPlayerTransform;
    
    static std::optional<System::RaceConfig::RaceSelections> sSelections;
};

__attribute__((export_name("alloc")))
u8* alloc(u32 size) { return new u8[size]; }

__attribute__((export_name("setButtons")))
void setButtons(u16 buttons, f32 stickX, f32 stickY) { sController->setButtons(buttons, stickX, stickY); }

__attribute__((export_name("calc")))
void* calc() {
    sceneMgr->calc();
    Pilz::updateTransform();
    return Pilz::sPlayerTransform;
}


int main() {
    JSlog("1");
    Pilz::init();
    JSlog("2");
    sceneMgr->changeScene(0);
    JSlog("3");
    System::KPadDirector::setController(sController);
    JSlog("4");

    setbuf(stdout, NULL);
    JSlog("initialized!");
    return 0;
}