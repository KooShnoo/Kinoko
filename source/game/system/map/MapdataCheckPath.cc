#include "game/system/map/MapdataCheckPath.hh"
#include <cassert>
#include <cstddef>

namespace System {

void MapdataCheckPath::read(EGG::Stream &stream) {
    m_start = stream.read_u8();
    m_size = stream.read_u8();
    stream.read(m_last.data(), 6);
    stream.read(m_next.data(), 6);
}

/// @brief performs DFS to find the next checkpath @param depth elements ahead
/// @addr{0x805150E0}
void MapdataCheckPath::findNextPath(u8 depth, MapdataCheckPathAccessor *accessor) {
    if (m_dfsDepth != 0xff) {
        return;
    }
    m_dfsDepth = depth;
    for (size_t i = 0; i < 6; i++) {
        u8 nextIdx = m_next[i];
        if (nextIdx == 0xff) {
            continue;
        }
        MapdataCheckPath *next = accessor->get(nextIdx);
        next->findNextPath(depth, accessor);
    }
    return;
}

u8 MapdataCheckPath::start() const {
    return m_start;
}

u8 MapdataCheckPath::size() const {
    return m_size;
}

std::array<u8, 6> MapdataCheckPath::last() const {
    return m_last;
}

std::array<u8, 6> MapdataCheckPath::next() const {
    return m_next;
}

u8 MapdataCheckPath::dfsDepth() const {
    return m_dfsDepth;
}

f32 MapdataCheckPath::oneOverCount() const {
    return m_oneOverCount;
}

void MapdataCheckPathAccessor::loadPaths() {
    assert(size() != 0);
    get(0)->findNextPath(0xff, this);
    u8 maxDepth = 0xff;
    for (size_t i = 0; i < size(); i = i + 1) {
        auto entry = get(i);
        if (maxDepth < entry->dfsDepth()) {
            entry = get(i);
            maxDepth = entry->dfsDepth();
        }
    }
    m_oneOverOnePlusMaxDepth = 1.0 / ((f32) maxDepth + 1.0);
}

MapdataCheckPathAccessor::MapdataCheckPathAccessor(const MapSectionHeader *header)
    : MapdataAccessorBase<MapdataCheckPath, MapdataCheckPath::SData>(header) {
    init(reinterpret_cast<const MapdataCheckPath::SData *>(m_sectionHeader + 1),
            parse<u16>(m_sectionHeader->count));
    loadPaths();
}

} // namespace System
