#pragma once

#include <game/system/map/MapdataAccessorBase.hh>

#include <egg/math/Vector.hh>

namespace System {

class MapdataCheckPoint;
struct LinkedCheckpoint {
    MapdataCheckPoint *checkpoint;
    EGG::Vector2f p0diff;
    EGG::Vector2f p1diff;
    f32 distance;
};

class MapdataCheckPoint {
public:
    struct SData {
        EGG::Vector2f left;
        EGG::Vector2f right;
        u8 jugemIndex;
        u8 lapCheck;
        u8 prevPt;
        u8 nextPt;
    };

    enum Completion {
        Completion_0,
        Completion_1,
        Completion_2,
    };

    MapdataCheckPoint(const SData *data);
    void read(EGG::Stream &stream);

    Completion getCompletion(const EGG::Vector3f &pos, f32 *distanceRatio) const;
    bool isPlayerFlagged(s32 playerIdx) const;
    void setPlayerFlags(s32 playerIdx);
    void resetFlags();
    // SData *data() const;
    EGG::Vector2f left() const;
    EGG::Vector2f right() const;
    u8 jugemIndex() const;
    u8 lapCheck() const;
    u8 prevPt() const;
    u8 nextPt() const;
    s32 nextCount() const;
    s32 prevCount() const;
    s32 id() const;
    MapdataCheckPoint *prevPoint(s32 i) const;
    MapdataCheckPoint *nextPoint(s32 i) const;

private:
    bool checkSector(const LinkedCheckpoint &next, const EGG::Vector2f &p0,
            const EGG::Vector2f &p1) const;
    bool checkDistanceRatio(const LinkedCheckpoint &next, const EGG::Vector2f &p0,
            const EGG::Vector2f &p1, f32 *distanceRatio) const;
    bool isOrientationNegative(const LinkedCheckpoint &next, const EGG::Vector2f &p0,
            const EGG::Vector2f &p1) const;
    bool isInCheckpoint(const LinkedCheckpoint &next, const EGG::Vector2f &p0,
            const EGG::Vector2f &p1, float *completion) const;
    Completion checkSectorAndDistanceRatio(const LinkedCheckpoint &next, const EGG::Vector2f &p0,
            const EGG::Vector2f &p1, float *distanceRatio) const;
    const SData *m_rawData;
    EGG::Vector2f m_left;
    EGG::Vector2f m_right;
    u8 m_jugemIndex; ///< index of respawn point associated with this checkpoint. players who die
                     ///< here will be respawned at this point.
    u8 m_lapCheck;
    u8 m_prevPt;
    u8 m_nextPt;
    u16 m_nextCount;
    u16 m_prevCount;
    EGG::Vector2f m_midpoint;
    EGG::Vector2f m_dir;
    u16 m_flags;
    u16 m_id;
    MapdataCheckPoint *m_prevPoints[6];
    LinkedCheckpoint m_nextPoints[6];
};

class MapdataCheckPointAccessor
    : public MapdataAccessorBase<MapdataCheckPoint, MapdataCheckPoint::SData> {
public:
    MapdataCheckPointAccessor(const MapSectionHeader *header);
    ~MapdataCheckPointAccessor() override;

private:
    void init();
    u8 m_lastKcpType;
    u16 m_finishLineCheckpointId;
    f32 m_meanTotalDistance;
};

} // namespace System
