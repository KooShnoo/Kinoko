#include "RaceConfig.hh"

#include "game/system/KPadDirector.hh"

#include <abstract/File.hh>
#include <game/system/GhostFile.hh>
#include <ranges>

namespace System {

/// @addr{0x8052DD40}
void RaceConfig::init() {
    m_raceScenario.init();
}

/// @addr{0x805302C4}
/// @details Normally we copy the menu scenario into the race scenario.
/// However, Kinoko doesn't support menus, so we use a callback.
void RaceConfig::initRace() {
    if (s_onInitCallback) {
        s_onInitCallback(this, s_onInitCallbackArg);
    }

    initControllers();
}

/// @addr{0x8052F4E8}
/// @brief Initializes the controllers.
/// @details This is normally scoped within RaceConfig::Scenario, but Kinoko doesn't support menus.
void RaceConfig::initControllers() {
#ifdef __clang__
            // clang does not support std::views::enumerate, a cpp23 feature :(
            for (auto [idx, player] : std::views::zip(std::views::iota(0), m_raceScenario.players)) {
#else
            for (auto [idx, player] : std::ranges::views::enumerate(m_raceScenario.players)) {
#endif
        switch (player.type) {
        case Player::Type::Ghost:
            initGhost(idx, player);
            break;
        case Player::Type::Local:
            KPadDirector::Instance()->pushHostPad(player.driftIsAuto);
            break;
        default:
            PANIC("Players must be either local or ghost!");
            break;
        }
    }
}

/// @addr{0x8052EEF0}
/// @brief Initializes the ghost.
/// @details This is normally scoped within RaceConfig::Scenario, but Kinoko doesn't support menus.
void RaceConfig::initGhost(size_t playerIdx, Player &player) {
    GhostFile ghost(m_ghosts.at(playerIdx));

    m_raceScenario.course = ghost.course();
    player.character = ghost.character();
    player.vehicle = ghost.vehicle();
    player.driftIsAuto = ghost.driftIsAuto();

    KPadDirector::Instance()->pushGhostPad(ghost.inputs(), ghost.driftIsAuto());
}

void RaceConfig::setGhost(const u8 *rkg, size_t playerIdx) {
    auto ghost = RawGhostFile(rkg);
    m_ghosts.insert_or_assign(playerIdx, ghost);
}

void RaceConfig::RegisterInitCallback(const InitCallback &callback, void *arg) {
    s_onInitCallback = callback;
    s_onInitCallbackArg = arg;
}

/// @addr{0x8052FE58}
RaceConfig *RaceConfig::CreateInstance() {
    ASSERT(!s_instance);
    s_instance = new RaceConfig;
    return s_instance;
}

/// @addr{0x8052FFE8}
void RaceConfig::DestroyInstance() {
    ASSERT(s_instance);
    auto *instance = s_instance;
    s_instance = nullptr;
    delete instance;
}

RaceConfig *RaceConfig::Instance() {
    return s_instance;
}

/// @addr{0x8053015C}
RaceConfig::RaceConfig() = default;

/// @addr{0x80530038}
RaceConfig::~RaceConfig() {
    if (s_instance) {
        s_instance = nullptr;
        WARN("RaceConfig instance not explicitly handled!");
    }
}

/// @addr{Inlined in 0x8052DD40}
void RaceConfig::Scenario::init() {
    this->course = Course::COURSE_MAX;
}

RaceConfig *RaceConfig::s_instance = nullptr; ///< @addr{0x809BD728}

/** @brief Host-agnostic way of initializing RaceConfig.
    The type of the first player *must* be set to either Local or Ghost.

    - If the type is Ghost, m_ghost must be set to a decompressed ghost file.

    - If the type is Local, the race scenario's course and the first player's character, vehicle,
    and driftIsAuto must be set.
*/
RaceConfig::InitCallback RaceConfig::s_onInitCallback = nullptr;

/// @brief The argument sent into the callback. This is expected to be reinterpret_casted.
void *RaceConfig::s_onInitCallbackArg = nullptr;

} // namespace System
