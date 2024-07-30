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

        u8 last[6];
        u8 next[6];
    };

    MapdataCheckPath(const SData *data);
    void read(EGG::Stream &stream);
    void findNextPath(u8 depth, MapdataCheckPathAccessor *accessor);

    u8 start() const;
    u8 size() const;
    std::array<u8, 6> last() const;
    std::array<u8, 6> next() const;
    u8 dfsDepth() const;
    f32 oneOverCount() const;

private:
    const SData *mpData;
    u8 m_start;
    u8 m_size;
    std::array<u8, 6> m_last;
    std::array<u8, 6> m_next;
    u8 m_dfsDepth;
    f32 m_oneOverCount;
};

class MapdataCheckPathAccessor
    : public MapdataAccessorBase<MapdataCheckPath, MapdataCheckPath::SData> {
public:
    MapdataCheckPathAccessor(const MapSectionHeader *header);
    ~MapdataCheckPathAccessor() override;

private:
    void loadPaths();

    f32 m_oneOverOnePlusMaxDepth;
};

} // namespace System
