#include "MapdataCheckPoint.hh"

#include "game/system/CourseMap.hh"

namespace System {

/// @addr{0x805154E4}
MapdataCheckPoint::MapdataCheckPoint(const SData *data)
    : m_rawData(data), m_nextCount(0), m_prevCount(0) {
    u8 *unsafeData = reinterpret_cast<u8 *>(const_cast<SData *>(data));
    EGG::RamStream stream = EGG::RamStream(unsafeData, sizeof(SData));
    read(stream);
    m_midpoint = EGG::Vector2f((left().x + right().x) / 2.0f, (left().y + right().y) / 2.0f);
    m_dir = EGG::Vector2f(left().y - right().y, right().x - left().x);
    m_dir.normalise();
}

void MapdataCheckPoint::read(EGG::Stream &stream) {
    m_left.read(stream);
    m_right.read(stream);
    m_jugemIndex = stream.read_u8();
    m_type = stream.read_s8();
    m_prevPt = stream.read_u8();
    m_nextPt = stream.read_u8();
}

bool MapdataCheckPoint::isPlayerFlagged(s32 /* playerIdx */) const {
    return m_flag;
}

void MapdataCheckPoint::setPlayerFlags(s32 /* playerIdx */) {
    m_flag = true;
}

void MapdataCheckPoint::resetFlags() {
    m_flag = false;
}

const EGG::Vector2f &MapdataCheckPoint::left() const {
    return m_left;
}

const EGG::Vector2f &MapdataCheckPoint::right() const {
    return m_right;
}

u8 MapdataCheckPoint::jugemIndex() const {
    return m_jugemIndex;
}

s8 MapdataCheckPoint::type() const {
    return m_type;
}

bool MapdataCheckPoint::isNormalCheckpoint() const {
    return m_type == NORMAL_CHECKPOINT;
}

bool MapdataCheckPoint::isFinishLine() const {
    return m_type == FINISH_LINE;
}

u16 MapdataCheckPoint::nextCount() const {
    return m_nextCount;
}

u16 MapdataCheckPoint::prevCount() const {
    return m_prevCount;
}

u16 MapdataCheckPoint::id() const {
    return m_id;
}

MapdataCheckPoint *MapdataCheckPoint::prevPoint(s32 i) const {
    return m_prevPoints[i];
}

MapdataCheckPoint *MapdataCheckPoint::nextPoint(s32 i) const {
    return m_nextPoints[i].checkpoint;
}

const LinkedCheckpoint &MapdataCheckPoint::nextLinked(s32 i) const {
    return m_nextPoints[i];
}

/// @brief computes @ref m_prevKcpId for each checkpoint.
/// @details @ref isPlayerFlagged indicates that a checkpoint has been visited, to prevent infinite
/// recursion.
/// https://decomp.me/scratch/MtF18
/// @addr{0x80515A6C}
void MapdataCheckPoint::linkPrevKcpIds(u8 prevKcpId) {
    m_prevKcpId = isNormalCheckpoint() ? prevKcpId : m_type;

    setPlayerFlags(0);
    for (size_t i = 0; i < nextCount(); i++) {
        MapdataCheckPoint *next = nextPoint(i);
        if (!next->isPlayerFlagged(0)) {
            next->linkPrevKcpIds(m_prevKcpId);
        }
    }
}

/// @addr{0x80512064}
f32 MapdataCheckPointAccessor::calculateMeanTotalDistanceRecursive(u16 ckptId) {
    f32 sumDist = 0.0f;
    MapdataCheckPoint *ckpt = get(ckptId);
    u16 nextCount = ckpt->nextCount();

    for (size_t i = 0; i < nextCount; i++) {
        const LinkedCheckpoint &linked = ckpt->nextLinked(i);
        sumDist += linked.distance;
        ckpt = ckpt->nextPoint(i);

        if (m_finishLineCheckpointId != ckpt->id()) {
            sumDist += calculateMeanTotalDistanceRecursive(ckpt->id());
        }
    }

    return sumDist / nextCount;
}

s8 MapdataCheckPointAccessor::lastKcpType() const {
    return m_lastKcpType;
}

/// @addr{80512370}
f32 MapdataCheckPointAccessor::calculateMeanTotalDistance() {
    return calculateMeanTotalDistanceRecursive(m_finishLineCheckpointId);
}

/// @brief find finish line and last key checkpoint indexes; also initCheckpointLinks for all
/// checkpoints
/// @addr{Inlined in 0x80515244} fake. not real. it's not in the base game. in the base
/// game, it's inlined into `init()`. i, kooshnoo, split it out because i wanted to.
// I'm not sure if this should be split; it feels like a separate function,
//  but idk if i can somehow prove nintendo wrote it as a separate function,
//  and i don't know if we even care what nintendo extracted to a separate function and what they
//  inlined anyways. would the PR reviewer kindly please advise, thank you
void MapdataCheckPointAccessor::findFinishAndLastKcp() {
    s8 lastKcpType = -1;
    s16 finishLineCheckpointId = -1;

    for (size_t ckptId = 0; ckptId < size(); ckptId++) {
        MapdataCheckPoint *checkpoint = get(ckptId);
        // @todo implement initCheckpointLinks
        // checkpoint->initCheckpointLinks(*this, ckptId);
        checkpoint = get(ckptId);

        if (checkpoint->isFinishLine()) {
            finishLineCheckpointId = ckptId;
        }

        lastKcpType = std::max(lastKcpType, checkpoint->type());
    }

    m_lastKcpType = lastKcpType;
    m_finishLineCheckpointId = finishLineCheckpointId;
}

/// @addr{0x80515244}
void MapdataCheckPointAccessor::init() {
    findFinishAndLastKcp();
    MapdataCheckPoint *finishLine = get(m_finishLineCheckpointId);
    finishLine->linkPrevKcpIds(0);
    // CourseMap::Instance()->clearSectorChecked(); //<- unnecessary, introduces UB maybe?
    //                                              //    (CourseMap::Instance()->m_checkpoint is
    //                                              uninitialized)
    //                                              //    would the PR reviewer kindly please
    //                                              advise, thank you
    // m_meanTotalDistance = calculateMeanTotalDistance(); //<- unused
}

MapdataCheckPointAccessor::MapdataCheckPointAccessor(const MapSectionHeader *header)
    : MapdataAccessorBase<MapdataCheckPoint, MapdataCheckPoint::SData>(header) {
    MapdataAccessorBase::init(
            reinterpret_cast<const MapdataCheckPoint::SData *>(m_sectionHeader + 1),
            parse<u16>(m_sectionHeader->count));
    init();
}

MapdataCheckPointAccessor::~MapdataCheckPointAccessor() = default;

} // namespace System
