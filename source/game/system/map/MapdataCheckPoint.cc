#include "MapdataCheckPoint.hh"

namespace System {

bool MapdataCheckPoint::isPlayerFlagged(s32 playerIdx) const {
    return m_flags & 1 << playerIdx;
}

void MapdataCheckPoint::setPlayerFlags(s32 playerIdx) {
    m_flags |= 1 << playerIdx;
}

void MapdataCheckPoint::resetFlags() {
    m_flags = 0;
}

// SData *MapdataCheckPoint::data() const {
//     return mpData;
// }

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

const LinkedCheckpoint &MapdataCheckPoint::nextPoint(s32 i) const {
    return m_nextPoints[i];
}

}