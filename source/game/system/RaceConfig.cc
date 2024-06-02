#include "RaceConfig.hh"

#include "game/system/GhostFile.hh"
#include "game/system/KPadDirector.hh"

#include <Common.hh>
#include <abstract/File.hh>
#include <host/System.hh>

namespace System {

/// @addr{0x8052DD40}
void RaceConfig::init() {
    m_raceScenario.init();
}

/// @addr{0x805302C4}
/// @details Normally we copy the menu scenario into the race scenario.
/// There's no menu scenario in Kinoko, so instead we initialize values here.
/// For now, we want to initialize solely based off the ghost file.
/// @todo don't. don't do that. initialize based on KSystem or somethign.
/// i think we have to create some sort of higher level abstract manager to respond to race events, 
/// like a race manager manger. and then kinoko will have one implementation, and Pilz will have another. 
/// gotta separate kinoko core and kinoko CLI, so that Pilz can depend on kinokoCore but not the CLI, ghost part
/// kinokoCore will still handle ghosts, but they have to be opted in by the manger. 
/// kinokoCLI's manager will opt into ghosts and feed ghossts to kinokoCOre, but we (Pilz) will not.
void RaceConfig::initRace() {
    // rather than reading from ghosts, this needs to read from some public interface, an API
    // of libkinoko, which kinokoCLI will use to test ghosts, and Pilz will use to play the game.
    // for now, Pilz is fighting kinokoCLI and putting all it's junk here b/c we hacnet extracted libkooko properly yet.
    m_raceScenario.playerCount = 1;

#ifdef __wasm
    Player &player = m_raceScenario.players[0];
    player.type = Player::Type::Local;
    m_raceScenario.course = Course::N64_DKs_Jungle_Parkway;
    player.character = Character::Funky_Kong;
    player.vehicle = Vehicle::Flame_Runner;
#else
    size_t size;
    const auto *testDirector = Host::KSystem::Instance().testDirector();
    u8 *rkg = Abstract::File::Load(testDirector->testCase().rkgPath.data(), size);
    m_ghost = rkg;
    delete[] rkg;
    GhostFile ghost(m_ghost);

    m_raceScenario.course = ghost.course();
    Player &player = m_raceScenario.players[0];
    player.character = ghost.character();
    player.vehicle = ghost.vehicle();
    player.type = Player::Type::Ghost;

    initGhostController(ghost);
#endif
}

/// @addr{0x8052F4E8}
/// @details This is normally scoped within RaceConfig::Scenario, but since Kinoko doesn't support
/// menus, we simplify and just initialize the controller here.
// i renamed this to ghostcontroller. mayb a bad idea.
void RaceConfig::initGhostController(const GhostFile &ghost) {
    KPadDirector::Instance()->setGhostPad(ghost.inputs(), ghost.driftIsAuto());
}

/// @addr{0x8052FE58}
RaceConfig *RaceConfig::CreateInstance() {
    assert(!s_instance);
    s_instance = new RaceConfig;
    return s_instance;
}

/// @addr{0x8052FFE8}
void RaceConfig::DestroyInstance() {
    assert(s_instance);
    delete s_instance;
    s_instance = nullptr;
}

RaceConfig *RaceConfig::Instance() {
    return s_instance;
}

/// @addr{0x8053015C}
RaceConfig::RaceConfig() = default;

/// @addr{0x80530038}
RaceConfig::~RaceConfig() = default;

/// @addr{Inlined in 0x8052DD40}
void RaceConfig::Scenario::init() {
    playerCount = 0;
    course = Course::GCN_Mario_Circuit;

    for (size_t i = 0; i < players.size(); ++i) {
        Player &player = players[i];
        player.character = Character::Mario;
        player.vehicle = Vehicle::Standard_Kart_M;
        player.type = Player::Type::None;
    }
}

RaceConfig *RaceConfig::s_instance = nullptr; ///< @addr{0x809BD728}

} // namespace System
