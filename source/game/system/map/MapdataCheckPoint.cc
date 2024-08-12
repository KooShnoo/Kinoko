#include "MapdataCheckPoint.hh"
#include "MapdataCheckPath.hh"
#include <Common.hh>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <egg/math/Vector.hh>
#include <game/system/CourseMap.hh>

namespace System {

MapdataCheckPoint::MapdataCheckPoint(const SData *data) : m_rawData(data) {
    u8 *unsafeData = reinterpret_cast<u8 *>(const_cast<SData *>(data));
    EGG::RamStream stream = EGG::RamStream(unsafeData, sizeof(SData));
    read(stream);
    // clang-format off
    m_midpoint = EGG::Vector2f(
        (left().x + right().x) / 2.0f, 
        (left().y + right().y) / 2.0f
    );
    m_dir = EGG::Vector2f(
        left().y - right().y,
        left().x - right().x
    );
    // clang-format on
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

/// @addr{0x80515624}
void MapdataCheckPoint::initCheckpointLinks(MapdataCheckPointAccessor &accessor, int id) {
    m_id = id;
    auto courseMap = CourseMap::Instance();
    // If the check point is the first in its group (prev == -1), it has multiple previous
    // checkpoints defined by its preceding checkpaths
    if (m_prevPt == 0xff) {
        // Finds the checkpath that contains the given checkpoint id
        // inline function?
        MapdataCheckPath *checkpath = nullptr;
        for (size_t i = 0; i < courseMap->getCheckPathCount(); i++) {
            checkpath = courseMap->getCheckPath(i);
            if (checkpath->isPointInPath(id)) {
                break;
            }
        }

        // assert(checkpath); // temporary, to see if we need all this vv // okay it failed lmao
        if (checkpath != nullptr) {
            m_prevCount = 0;
            for (size_t i = 0; i < 6; i++) {
                u16 prevID = checkpath->getPrev(i);
                if (prevID == 0xff) {
                    continue;
                }
                MapdataCheckPath *prev = courseMap->checkPath()->get(prevID);
                m_prevPoints[i] = accessor.get(prev->end());
                m_prevCount++;
            }
        }
    } else {
        m_prevPoints[0] = accessor.get(m_prevPt);
        m_prevCount++;
    }
    // Calculate the quadrilateral's nextCheckpoint. If the checkpoint is the last in its group, it
    // can have multiple quadrilaterals (and nextCheckpoint) which are determined by its next
    // path(s)
    if (m_nextPt == 0xff) {
        // Finds the checkpath that contains the given checkpoint id
        MapdataCheckPath *checkpath = nullptr;
        for (size_t i = 0; i < courseMap->getCheckPathCount(); i++) {
            checkpath = courseMap->getCheckPath(i);
            if (checkpath->isPointInPath(id)) {
                break;
            }
        }

        // assert(checkpath); // temporary, to see if we need all this vv // okay it failed lmao
        if (checkpath != nullptr) {
            m_nextCount = 0;
            for (size_t i = 0; i < 6; i++) {
                u16 nextID = checkpath->getNext(i);
                if (nextID == 0xff) {
                    continue;
                }
                MapdataCheckPath *next = courseMap->checkPath()->get(nextID);
                m_nextPoints[i].checkpoint = accessor.get(next->end());
                m_nextCount++;
            }
        }
    } else {
        m_nextPoints[0].checkpoint = accessor.get(m_nextPt);
        m_nextCount++;
    }

    // Form the checkpoint's quadrilateral(s)
    for (size_t i = 0; i < 6; i++) {
        if (i < nextCount()) {
            auto next = nextPoint(i);
            m_nextPoints[i].distance = (next->m_midpoint - m_midpoint).normalise();

            m_nextPoints[i].p0diff =
                    EGG::Vector2f(next->left().x - left().x, next->left().y - left().y);
            m_nextPoints[i].p1diff =
                    EGG::Vector2f(next->right().x - right().x, next->right().y - right().y);
        } // else initalize to zero (pointless)
    }
}

/// @addr{0x80510D7C}
/// @see MapdataCheckPoint::checkSectorAndCheckpointCompletion
MapdataCheckPoint::Completion MapdataCheckPoint::getCompletion(const EGG::Vector3f &pos,
        float *completion) const {
    EGG::Vector2f p1 = EGG::Vector2f(right().x, right().y);
    p1.y = pos.z - p1.y;
    p1.x = pos.x - p1.x;

    for (s32 nextIdx = 0; nextIdx < m_nextCount; nextIdx++) {
        EGG::Vector2f p0(m_nextPoints[nextIdx].checkpoint->left().x,
                m_nextPoints[nextIdx].checkpoint->left().y);
        p0.y = pos.z - p0.y;
        p0.x = pos.x - p0.x;
        MapdataCheckPoint::Completion result =
                checkSectorAndCheckpointCompletion(m_nextPoints[nextIdx], p0, p1, completion);

        if (result == Completion_1) {
            continue;
        } else {
            return result;
        }
    }

    return Completion_1;
}

bool MapdataCheckPoint::isPlayerFlagged(s32 playerIdx) const {
    return m_flags & 1 << playerIdx;
}

void MapdataCheckPoint::setPlayerFlags(s32 playerIdx) {
    m_flags |= 1 << playerIdx;
}

void MapdataCheckPoint::resetFlags() {
    m_flags = 0;
}

EGG::Vector2f MapdataCheckPoint::left() const {
    return m_left;
}

EGG::Vector2f MapdataCheckPoint::right() const {
    return m_right;
}

u8 MapdataCheckPoint::jugemIndex() const {
    return m_jugemIndex;
}

s8 MapdataCheckPoint::type() const {
    return m_type;
}

bool MapdataCheckPoint::isNormalCheckpoint() const {
    return type() == NORMAL_CHECKPOINT;
}

bool MapdataCheckPoint::isFinishLine() const {
    return type() == FINISH_LINE;
}

u8 MapdataCheckPoint::prevPt() const {
    return m_prevPt;
}

u8 MapdataCheckPoint::nextPt() const {
    return m_nextPt;
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
    m_prevKcpId = isNormalCheckpoint() ? prevKcpId : this->type();

    setPlayerFlags(0);
    for (size_t i = 0; i < nextCount(); i++) {
        MapdataCheckPoint *next = nextPoint(i);
        if (next->isPlayerFlagged(0)) {
            continue;
        }
        next->linkPrevKcpIds(m_prevKcpId);
    }
}

/// @brief Returns true if player is between the two sides of the checkpoint quad, otherwise false
bool MapdataCheckPoint::checkSector(const LinkedCheckpoint &next, const EGG::Vector2f &p0,
        const EGG::Vector2f &p1) const {
    if (-(next.p0diff.y) * p0.x + next.p0diff.x * p0.y < 0.0f) {
        return false;
    }

    if (next.p1diff.y * p1.x - next.p1diff.x * p1.y < 0.0f) {
        return false;
    }

    return true;
}

/// @brief Updates @param checkpointCompletion\, which is the percentage of distance
/// the player has traveled through the checkpoint quad
/// @return True if 0 <= checkpointCompletion <= 1, meaning the player is between this checkpoint
/// line and the next; otherwise, false
/// @addr{Inlined in 0x80510C74}
bool MapdataCheckPoint::checkCheckpointCompletion(const LinkedCheckpoint &next,
        const EGG::Vector2f &p0, const EGG::Vector2f &p1, float *checkpointCompletion) const {
    f32 d1 = m_dir.dot(p1);
    f32 d2 = -(next.checkpoint->m_dir.dot(p0));
    // This is where the divide by zero thing happens @todo
    f32 checkpointCompletion_ = d1 / (d1 + d2);
    *checkpointCompletion = checkpointCompletion_;
    printf("checkpointCompletion: %+02.4f, _:%+02.4f, d1:%+02.4f, d2:%+02.4f, d1+d2:%+02.4f ",
            *checkpointCompletion, checkpointCompletion_, d1, d2, d1 + d2);
    return (checkpointCompletion_ >= 0.0f && checkpointCompletion_ <= 1.0f);
}

/// @brief Calls both @ref checkSector and @ref checkCheckpointCompletion; updates @param
/// checkpointCompletion
/// @addr{0x80510C74}
MapdataCheckPoint::Completion MapdataCheckPoint::checkSectorAndCheckpointCompletion(
        const LinkedCheckpoint &next, const EGG::Vector2f &p0, const EGG::Vector2f &p1,
        float *checkpointCompletion) const {
    if (!checkSector(next, p0, p1)) {
        return Completion_1;
    }

    return checkCheckpointCompletion(next, p0, p1, checkpointCompletion) ? Completion_0 :
                                                                           Completion_2;
}

/// @addr{0x80512064}
f32 MapdataCheckPointAccessor::calculateMeanTotalDistanceRecursive(u16 ckptId) {
    f32 sumDist = 0.0f;
    MapdataCheckPoint *ckpt = get(ckptId);
    u16 n = ckpt->nextCount();
    for (size_t i = 0; i < n; i++) {
        const LinkedCheckpoint &linked = ckpt->nextLinked(i);
        sumDist += linked.distance;
        ckpt = ckpt->nextPoint(i);
        if (m_finishLineCheckpointId == ckpt->id()) {
            continue;
        }
        // ckpt = ckpt->nextPoint(i);
        sumDist += calculateMeanTotalDistanceRecursive(ckpt->id());
    }

    return sumDist / n;
}

s8 MapdataCheckPointAccessor::lastKcpType() const {
    return m_lastKcpType;
}

/// @addr{80512370}
f32 MapdataCheckPointAccessor::calculateMeanTotalDistance() {
    assert(size() != 0);
    return calculateMeanTotalDistanceRecursive(m_finishLineCheckpointId);
}

/// @brief find finish line and last key checkpoint indexes
/// @addr{Inlined in 0x80515244} fake. not real. it's not in the base game. in the base
/// game, it's inlined into `init()`. i, kooshnoo, split it out because i wanted to.
void MapdataCheckPointAccessor::findFinishAndLastKcp() {
    s8 lastKcpType = -1;
    s16 finishLineCheckpointId = -1;
    for (size_t ckptId = 0; ckptId < size(); ckptId++) {
        MapdataCheckPoint *lastCheckpoint = get(ckptId);
        lastCheckpoint->initCheckpointLinks(*this, ckptId);
        lastCheckpoint = get(ckptId);
        if (lastCheckpoint->isFinishLine()) {
            finishLineCheckpointId = ckptId;
        }
        if (lastKcpType > lastCheckpoint->type()) {
            lastKcpType = lastCheckpoint->type();
        }
    }
    m_lastKcpType = lastKcpType;
    m_finishLineCheckpointId = finishLineCheckpointId;
}

/// @addr{0x80515244}
void MapdataCheckPointAccessor::init() {
    assert(size() != 0);

    findFinishAndLastKcp();
    MapdataCheckPoint *finishLine = get(m_finishLineCheckpointId);
    finishLine->linkPrevKcpIds(0);
    CourseMap::Instance()->clearSectorChecked();
    m_meanTotalDistance = calculateMeanTotalDistance();
}

MapdataCheckPointAccessor::MapdataCheckPointAccessor(const MapSectionHeader *header)
    : MapdataAccessorBase<MapdataCheckPoint, MapdataCheckPoint::SData>(header) {
    MapdataAccessorBase::init(
            reinterpret_cast<const MapdataCheckPoint::SData *>(m_sectionHeader + 1),
            parse<u16>(m_sectionHeader->count));
    assert(size() != 0);
    init();
}

MapdataCheckPointAccessor::~MapdataCheckPointAccessor() = default;

} // namespace System
