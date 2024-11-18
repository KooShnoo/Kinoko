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
    
    struct SData {
        u16 pointCount;
        u8 setting1;
        u8 setting2;
        Point points[];
    };

    MapdataPointInfo(const SData *data);

    void read(EGG::Stream &stream);

private:
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
};

} // namespace System
