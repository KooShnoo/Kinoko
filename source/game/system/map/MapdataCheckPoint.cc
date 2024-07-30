#include "MapdataCheckPoint.hh"
#include <Common.hh>
#include <cstddef>

namespace System {

void MapdataCheckPoint::read(EGG::Stream &stream) {
    m_left.read(stream);
    m_right.read(stream);
    m_jugemIndex = stream.read_u8();
    m_lapCheck = stream.read_u8();
    m_prevPt = stream.read_u8();
    m_nextPt = stream.read_u8();
}

/// @addr{0x80510D7C}
/// @see MapdataCheckPoint::checkSectorAndDistanceRatio
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
                checkSectorAndDistanceRatio(m_nextPoints[nextIdx], p0, p1, completion);

        if (result == Completion_1) {
            continue;
        }
        return result;
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

u8 MapdataCheckPoint::lapCheck() const {
    return m_lapCheck;
}

u8 MapdataCheckPoint::prevPt() const {
    return m_prevPt;
}

u8 MapdataCheckPoint::nextPt() const {
    return m_nextPt;
}

s32 MapdataCheckPoint::nextCount() const {
    return m_nextCount;
}

s32 MapdataCheckPoint::prevCount() const {
    return m_prevCount;
}

s32 MapdataCheckPoint::id() const {
    return m_id;
}

MapdataCheckPoint *MapdataCheckPoint::prevPoint(s32 i) const {
    return m_prevPoints[i];
}

MapdataCheckPoint *MapdataCheckPoint::nextPoint(s32 i) const {
    return m_nextPoints[i].checkpoint;
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

/// @brief Updates distanceRatio, which is the "checkpoint completion" or the percentage of distance
/// the player has traveled through the checkpoint quad
/// @return True if 0 <= distanceRatio <= 1, meaning the player is between this checkpoint line and
/// the next; otherwise, false
bool MapdataCheckPoint::checkDistanceRatio(const LinkedCheckpoint &next, const EGG::Vector2f &p0,
        const EGG::Vector2f &p1, float *distanceRatio) const {
    f32 d1 = m_dir.dot(p1);
    f32 d2 = -(next.checkpoint->m_dir.dot(p0));
    // This is where the divide by zero thing happens
    f32 distanceRatio_ = d1 / (d1 + d2);
    *distanceRatio = distanceRatio_;
    return (distanceRatio_ >= 0.0f && distanceRatio_ <= 1.0f);
}

/// @brief Calls both of the above functions and updates distanceRatio
/// @return - Completion_0 if player is inside the checkpoint quad
/// - Completion_1 if player is not between the sides of the quad (may still be between this
/// checkpoint and next); player is likely in a different checkpoint group
/// - Completion_2 if player is between the sides of the quad, but NOT between this checkpoint and
/// next; player is likely in the same checkpoint group
MapdataCheckPoint::Completion MapdataCheckPoint::checkSectorAndDistanceRatio(
        const LinkedCheckpoint &next, const EGG::Vector2f &p0, const EGG::Vector2f &p1,
        float *distanceRatio) const {
    if (!checkSector(next, p0, p1)) {
        return Completion_1;
    }

    return checkDistanceRatio(next, p0, p1, distanceRatio) ? Completion_0 : Completion_2;
}

/// @addr{0x80515244}
/// @todo TODO complete this TODO @todo
void MapdataCheckPointAccessor::init() {
    assert(m_entryCount != 0);
    // u16 finishLineCheckpointId;
    // /* find finish line and last key checkpoint indexes */
    // for (size_t ckptId = 0; ckptId < m_entryCount; ckptId++) {
    //     auto lastCheckpoint = get(ckptId);

    // }
}

MapdataCheckPointAccessor::MapdataCheckPointAccessor(const MapSectionHeader *header)
    : MapdataAccessorBase<MapdataCheckPoint, MapdataCheckPoint::SData>(header) {}

} // namespace System
