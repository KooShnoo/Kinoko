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
MapdataCheckPoint::Completion MapdataCheckPoint::checkSectorAndDistanceRatio(
        const EGG::Vector3f &pos, float *completion) const {
    // const LinkedCheckpoint *next = &m_nextPoints[0];
    // bool bIs02 = false;
    EGG::Vector2f p1 = EGG::Vector2f(right().x, right().y);
    p1.y = pos.z - p1.y;
    p1.x = pos.x - p1.x;
    bool bIs02 = false;

    for (s32 nextIdx = 0; nextIdx < m_nextCount; nextIdx++) {
        EGG::Vector2f p0(m_nextPoints[nextIdx].checkpoint->left().x,
                m_nextPoints[nextIdx].checkpoint->left().y);
        p0.y = pos.z - p0.y;
        p0.x = pos.x - p0.x;
        MapdataCheckPoint::Completion result =
                checkSectorAndDistanceRatio_(m_nextPoints[nextIdx], p0, p1, completion);

        switch (result) {
        case Completion_0:
            return Completion_0;
        case Completion_1:
            break;
        case Completion_2:
            bIs02 = true;
            break;
        }
    }

    return bIs02 ? Completion_2 : Completion_1;
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

// Auto-inlines organically
// BAD NAME!
inline bool MapdataCheckPoint::isOrientationNegative(const LinkedCheckpoint &next,
        const EGG::Vector2f &p0, const EGG::Vector2f &p1) const {
    if (-(next.p0diff.y) * p0.x + next.p0diff.x * p0.y < 0.0f) {
        return false;
    }

    if (next.p1diff.y * p1.x - next.p1diff.x * p1.y < 0.0f) {
        return false;
    }

    return true;
}

// Auto-inlines organically
// BAD NAME!
inline bool MapdataCheckPoint::isInCheckpoint(const LinkedCheckpoint &next, const EGG::Vector2f &p0,
        const EGG::Vector2f &p1, float *completion) const {
    f32 d1 = m_dir.dot(p1);
    f32 d2 = -(next.checkpoint->m_dir.dot(p0));
    f32 completion_ = d1 / (d1 + d2);
    *completion = completion_;
    return (completion_ >= 0.0f && completion_ <= 1.0f);
}

/// @addr{0x80510C74}
inline MapdataCheckPoint::Completion MapdataCheckPoint::checkSectorAndDistanceRatio_(
        const LinkedCheckpoint &next, const EGG::Vector2f &p0, const EGG::Vector2f &p1,
        float *completion) const {
    if (!isOrientationNegative(next, p0, p1)) {
        return Completion_1;
    }

    return isInCheckpoint(next, p0, p1, completion) ? Completion_0 : Completion_2;
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
    : MapdataAccessorBase<MapdataCheckPoint, MapdataCheckPoint::SData>(header) {

    }

} // namespace System
