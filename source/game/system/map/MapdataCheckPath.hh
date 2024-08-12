#include "game/system/map/MapdataAccessorBase.hh"
#include <array>
#include <egg/util/Stream.hh>

namespace System {

class MapdataCheckPathAccessor;
class MapdataCheckPath {
public:
    struct SData {
        u8 start;
        u8 size;

        u8 prev[6];
        u8 next[6];
    };

    MapdataCheckPath(const SData *data);
    void read(EGG::Stream &stream);
    void findDepth(s8 depth, const MapdataCheckPathAccessor &accessor);
    u16 getPrev(u16 i) const;
    u16 getNext(u16 i) const;

    u8 start() const;
    u8 size() const;
    u8 end() const;
    std::array<u8, 6> prev() const;
    std::array<u8, 6> next() const;
    u8 depth() const;
    f32 oneOverCount() const;
    bool isPointInPath(u16 checkpointId) const;

private:
    const SData *m_rawData;
    u8 m_start;               ///< index of the first checkpoint in this checkpath
    u8 m_size;                ///< number of checkpoints in this checkpath
    std::array<u8, 6> m_prev; ///< indices of previous connected checkpaths
    std::array<u8, 6> m_next; ///< indices of next connected checkpaths
    s8 m_depth; ///< number of checkpaths away from the first checkpath (i.e. distance from the
                ///< start)
    f32 m_oneOverCount;
};

class MapdataCheckPathAccessor
    : public MapdataAccessorBase<MapdataCheckPath, MapdataCheckPath::SData> {
public:
    MapdataCheckPathAccessor(const MapSectionHeader *header);
    ~MapdataCheckPathAccessor() override;

    f32 lapProportion();

    MapdataCheckPath *findCheckpathForCheckpoint(u16 checkpointId);

private:
    void loadPaths();

    f32 m_lapProportion; ///< minimum proportion of a lap a checkpath can be; calculated as
                         ///< 1/(maxDepth+1). another way to think of it: maxDepth+1 is the number
                         ///< of checkpaths in the longest route through the course, where longest
                         ///< means *most checkpaths traversed*, not most _distance_ traversed. so
                         ///< if one plans their route to hit the most checkpaths possible (no
                         ///< backtracking), they hit maxDepth+1 checkpaths, and each checkpath is
                         ///< `lapProportion`% of the total checkpaths on the route.
};

} // namespace System
