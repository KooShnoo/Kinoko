#include "MapdataPointInfo.hh"

#include <vector>

namespace System {

void MapdataPointInfo::Point::read(EGG::Stream & stream) {
    pos.read(stream);
    setting1 = stream.read_u8();
    setting2 = stream.read_u8();
}

MapdataPointInfo::MapdataPointInfo(const SData *data) : m_rawData(data) {
    u8 *unsafeData = reinterpret_cast<u8 *>(const_cast<SData *>(data));
    EGG::RamStream stream = EGG::RamStream(unsafeData, sizeof(SData));
    read(stream);
}

void MapdataPointInfo::read(EGG::Stream & stream) {
    m_pointCount = stream.read_u16();
    m_setting1 = stream.read_u8();
    m_setting2 = stream.read_u8();
    m_points.reserve(m_pointCount + 4 /* just rezesrve a little more, just in case, it can't hurt */);
    for (size_t i = 0; i < m_pointCount; i++) {
        m_points.emplace_back().read(stream);
    }
}

MapdataPointInfoAccessor::MapdataPointInfoAccessor(const MapSectionHeader *header)
    : MapdataAccessorBase<MapdataPointInfo, MapdataPointInfo::SData>(header) {
    init(reinterpret_cast<const MapdataPointInfo::SData *>(m_sectionHeader + 1),
            parse<u16>(m_sectionHeader->count));
}

MapdataPointInfoAccessor::~MapdataPointInfoAccessor() = default;

} // namespace System
