#pragma once

#include "game/system/map/MapdataAccessorBase.hh"

#include <egg/math/Vector.hh>
#include <vector>

namespace System {

class MapdataPointInfo {
public:
    struct Point {
        EGG::Vector3f pos;
        u8 setting1;
        u8 setting2;

        void read(EGG::Stream &stream);
    };
    STATIC_ASSERT(sizeof(Point) == 0x10);

    struct SData {
        u16 pointCount;
        u8 setting1;
        u8 setting2;
        Point points[];
    };
    STATIC_ASSERT(sizeof(SData) == 0x4);

    MapdataPointInfo(const SData *data);

    void read(EGG::Stream &stream);

    // private:
    const SData *m_rawData;
    u16 m_pointCount; ///< number of points comprising this route
    u8 m_setting1;
    u8 m_setting2;
    std::vector<Point> m_points;
};

class MapdataPointInfoAccessor
    : public MapdataAccessorBase<MapdataPointInfo, MapdataPointInfo::SData> {
public:
    MapdataPointInfoAccessor(const MapSectionHeader *header);
    ~MapdataPointInfoAccessor() override;
    void init(const MapdataPointInfo::SData *start, u16 count) {
        if (count != 0) {
            m_entryCount = count;
            m_entries = new MapdataPointInfo *[count];
        }

        auto *rawData = start;
        for (u16 i = 0; i < count; ++i) {
            auto *route = new MapdataPointInfo(rawData);
            m_entries[i] = route;
            // @todo unsafe pointer arithmetic
            rawData++;
            rawData = reinterpret_cast<MapdataPointInfo::SData *>(
                    reinterpret_cast<MapdataPointInfo::Point *>(&route->m_points) +
                    route->m_pointCount);
        }
    }
};

} // namespace System
