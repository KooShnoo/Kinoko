#include "RaceManager.hh"

#include "game/system/CourseMap.hh"
#include "game/system/KPadDirector.hh"
#include "game/system/map/MapdataStartPoint.hh"
#include <Common.hh>
#include <cfloat>
#include <game/kart/KartObjectManager.hh>
#include <game/system/map/MapdataCheckPoint.hh>

#include <algorithm>
#include <cassert>

namespace System {

/// @addr{0x80532F88}
void RaceManager::init() {
    m_player.init();
}

/// @addr{0x805362DC}
/// @todo When expanding to other gamemodes, we will need to pass the player index
void RaceManager::findKartStartPoint(EGG::Vector3f &pos, EGG::Vector3f &angles) {
    u32 placement = 1;
    u32 playerCount = 1;
    u32 startPointIdx = 0;

    MapdataStartPoint *kartpoint = CourseMap::Instance()->getStartPoint(startPointIdx);

    if (kartpoint) {
        kartpoint->findKartStartPoint(pos, angles, placement - 1, playerCount);
    } else {
        pos.setZero();
        angles = EGG::Vector3f::ex;
    }
}

/// @addr{0x80584334}
const MapdataJugemPoint *RaceManager::jugemPoint() {
    return m_raceMode.jugemPoint();
}

/// @addr{0x805331B4}
void RaceManager::calc() {
    constexpr u16 STAGE_INTRO_DURATION = 172;

    switch (m_stage) {
    case Stage::Intro:
        if (++m_introTimer >= STAGE_INTRO_DURATION) {
            m_stage = Stage::Countdown;
            KPadDirector::Instance()->startGhostProxies();
        }
        break;
    case Stage::Countdown:
        if (++m_timer >= STAGE_COUNTDOWN_DURATION) {
            m_stage = Stage::Race;
        }
        break;
    case Stage::Race:
        ++m_timer;
        break;
    default:
        break;
    }
}

/// @addr{0x80536230}
bool RaceManager::isStageReached(Stage stage) const {
    return static_cast<std::underlying_type_t<Stage>>(m_stage) >=
            static_cast<std::underlying_type_t<Stage>>(stage);
}

/// @addr{0x80533090}
int RaceManager::getCountdownTimer() const {
    return STAGE_COUNTDOWN_DURATION - m_timer;
}

const RaceManagerPlayer &RaceManager::player() const {
    return m_player;
}

RaceManager::Stage RaceManager::stage() const {
    return m_stage;
}

/// @addr{0x80532084}
RaceManager *RaceManager::CreateInstance() {
    assert(!s_instance);
    s_instance = new RaceManager;
    return s_instance;
}

RaceManager *RaceManager::Instance() {
    return s_instance;
}

/// @addr{0x805320D4}
void RaceManager::DestroyInstance() {
    assert(s_instance);
    delete s_instance;
    s_instance = nullptr;
}

/// @addr{0x805327A0}
RaceManager::RaceManager() : m_player(0, 3), m_stage(Stage::Intro), m_introTimer(0), m_timer(0) {}

/// @addr{0x80532E3C}
RaceManager::~RaceManager() = default;

/// @addr{0x80533ED8}
RaceManagerPlayer::RaceManagerPlayer(u8 idx, u8 lapCount)
    : m_idx(idx), m_checkpointId(0), m_raceCompletion(0.0f), m_checkpointFactor(-1.0f),
      m_checkpointStartLapCompletion(0.0f), m_lapCompletion(0.999999f) {

    CourseMap::Instance()->getCheckPointCount();

    m_lapFinishTimes = std::vector<Timer>(lapCount);
    m_inputs = &KPadDirector::Instance()->playerInput();
}

/// @addr{0x80534194}
void RaceManagerPlayer::init() {
    auto courseMap = CourseMap::Instance();
    assert(courseMap);
    bool hasCkpt = courseMap->getCheckPointCount() == 0;
    if (hasCkpt) {
        hasCkpt = courseMap->getCheckPathCount() != 0;
    }
    if (hasCkpt) {
        auto pos = Kart::KartObjectManager::Instance()->object(m_idx)->pos();
        f32 distanceRatio = 0.0f;
        s16 sector = courseMap->findSector(m_idx, pos, 0, &distanceRatio, true);
        m_checkpointId = std::max<s16>(0, sector);
        auto *ckpt = courseMap->getCheckPoint(m_checkpointId);
        m_respawn = ckpt->jugemIndex();
    } else {
        m_respawn = 0;
    }
    m_frameCounter = 0;
    m_bInRace = true;
}

const KPad *RaceManagerPlayer::inputs() const {
    return m_inputs;
}

RaceManager *RaceManager::s_instance = nullptr; ///< @addr{0x809BD730}

} // namespace System
