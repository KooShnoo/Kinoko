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
    static constexpr s8 NORMAL_CHECKPOINT = -1; ///< only used for respawn calculations
    static constexpr s8 START_FINISH_LINE = 0;  ///< triggers a lap count

    struct SData {
        EGG::Vector2f left;
        EGG::Vector2f right;
        u8 jugemIndex;
        s8 lapCheck;
        u8 prevPt;
        u8 nextPt;
    };

    enum Completion {
        Completion_0, ///< if player is inside the checkpoint quad
        Completion_1, ///< if player is not between the sides of the quad (may still be between this
                      ///< checkpoint and next); player is likely in a different checkpoint group
        Completion_2, ///< if player is between the sides of the quad, but NOT between this
                      ///< checkpoint and
                      ///< next; player is likely in the same checkpoint group
    };

    MapdataCheckPoint(const SData *data);
    void read(EGG::Stream &stream);

    Completion getCompletion(const EGG::Vector3f &pos, f32 *distanceRatio) const;
    bool isPlayerFlagged(s32 playerIdx) const;
    void setPlayerFlags(s32 playerIdx);
    void resetFlags();
    EGG::Vector2f left() const;
    EGG::Vector2f right() const;
    u8 jugemIndex() const;
    s8 type() const;
    bool isNormalCheckpoint() const;
    bool isStartFinishLine() const;
    u8 prevPt() const;
    u8 nextPt() const;
    u16 nextCount() const;
    u16 prevCount() const;
    u16 id() const;
    MapdataCheckPoint *prevPoint(s32 i) const;
    MapdataCheckPoint *nextPoint(s32 i) const;

private:
    void linkPrevKcpIds(u8 prevKcpId);
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
    /// either:
    /// - a @ref `NORMAL_CHECKPOINT` (0) used to calulate respawns,
    /// - a @ref `START_FINISH_LINE` (-1) which updates the lap count when crossed, or
    /// - a "key checkpoint" (1-127) used to ensure racers travel around the entire
    /// course before proceeding to the next lap. the type value represents the index,
    /// i.e. racers must pass checkpoint with @ref `m_type` 1, then 2, then 3 etc..
    s8 m_type;
    u8 m_prevPt;
    u8 m_nextPt;
    u16 m_nextCount;
    u16 m_prevCount;
    EGG::Vector2f m_midpoint;
    EGG::Vector2f m_dir;
    u16 m_flags; ///< visited flag, for recursive fucntions
    u16 m_id;
    u8 prevKcpId;
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
