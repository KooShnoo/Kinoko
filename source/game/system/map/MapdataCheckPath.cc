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

/// @brief performs DFS to find the depth of this check path? 
/// @param depth where to start ? ig? idk tbh
/// https://decomp.me/scratch/P4Rpz
/// @addr{0x805150E0}
void MapdataCheckPath::findDepth(s8 depth, MapdataCheckPathAccessor *accessor) {
    if (m_dfsDepth != -1) {
        return;
    }

    m_dfsDepth = depth;

    for (u16 i = 0; i < 6; i++) {
        u16 nextID = getNext(i);
        if (nextID == 0xff) {
            continue;
        }
        MapdataCheckPath *next = accessor->get(nextID);
        next->findDepth(depth + 1, accessor);
    }
}

u16 MapdataCheckPath::getNext(u16 i) const {
    return mpData->next[i]; 
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

/// @addr{Inlined in 0x8051377C}
void MapdataCheckPathAccessor::loadPaths() {
    assert(size() != 0);
    get(0)->findDepth(0xff, this);
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
