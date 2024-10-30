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

class MapdataCheckPointAccessor;
class MapdataCheckPoint {
public:
    static constexpr s8 NORMAL_CHECKPOINT = -1; ///< only used for picking respawn position
    static constexpr s8 FINISH_LINE = 0;        ///< triggers a lap change; also the starting line

    struct SData {
        EGG::Vector2f left;
        EGG::Vector2f right;
        u8 jugemIndex;
        s8 lapCheck;
        u8 prevPt;
        u8 nextPt;
    };

    enum SectorOccupancy {
        InsideSector,  ///< if player is inside the checkpoint quad
        OutsideSector, ///< if player is not between the sides of the quad (may still be between
                       ///< this checkpoint and next); player is likely in a different checkpoint
                       ///< group
        OutsideSector_BetweenSides, ///< if player is between the sides of the quad, but NOT between
                                    ///< this checkpoint and next; player is likely in the same
                                    ///< checkpoint group
    };

    MapdataCheckPoint(const SData *data);
    void read(EGG::Stream &stream);

    void initCheckpointLinks(MapdataCheckPointAccessor &accessor, int id);
    [[nodiscard]] SectorOccupancy checkSectorAndCheckpointCompletion(const EGG::Vector3f &pos,
            f32 *checkpointCompletion) const;
    bool isPlayerFlagged(s32 playerIdx) const;
    void setPlayerFlags(s32 playerIdx);
    void resetFlags();

    /// @beginGetters
    [[nodiscard]] EGG::Vector2f left() const;
    [[nodiscard]] EGG::Vector2f right() const;
    [[nodiscard]] u8 jugemIndex() const;
    [[nodiscard]] s8 type() const;
    [[nodiscard]] bool isNormalCheckpoint() const;
    [[nodiscard]] bool isFinishLine() const;
    [[nodiscard]] u8 prevPt() const;
    [[nodiscard]] u8 nextPt() const;
    [[nodiscard]] u16 nextCount() const;
    [[nodiscard]] u16 prevCount() const;
    [[nodiscard]] u16 id() const;
    [[nodiscard]] MapdataCheckPoint *prevPoint(s32 i) const;
    [[nodiscard]] MapdataCheckPoint *nextPoint(s32 i) const;
    [[nodiscard]] const LinkedCheckpoint &nextLinked(s32 i) const;
    /// @endGetters

    void linkPrevKcpIds(u8 prevKcpId);

private:
    [[nodiscard]] bool checkSector(const LinkedCheckpoint &next, const EGG::Vector2f &p0,
            const EGG::Vector2f &p1) const;
    [[nodiscard]] bool checkCheckpointCompletion(const LinkedCheckpoint &next,
            const EGG::Vector2f &p0, const EGG::Vector2f &p1, f32 *checkpointCompletion) const;
    [[nodiscard]] bool isOrientationNegative(const LinkedCheckpoint &next, const EGG::Vector2f &p0,
            const EGG::Vector2f &p1) const;
    [[nodiscard]] bool isInCheckpoint(const LinkedCheckpoint &next, const EGG::Vector2f &p0,
            const EGG::Vector2f &p1, float *completion) const;
    [[nodiscard]] SectorOccupancy checkSectorAndCheckpointCompletion_(const LinkedCheckpoint &next,
            const EGG::Vector2f &p0, const EGG::Vector2f &p1, float *checkpointCompletion) const;
    const SData *m_rawData;
    EGG::Vector2f m_left;
    EGG::Vector2f m_right;
    s8 m_jugemIndex; ///< index of respawn point associated with this checkpoint. players who die
                     ///< here will be respawned at this point.
    /// either:
    /// - a @ref `NORMAL_CHECKPOINT` (0) used to calulate respawns,
    /// - a @ref `FINISH_LINE` (-1) which updates the lap count when crossed, or
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
    bool m_flag; ///< visited flag, for recursive fucntions.
    u16 m_id;
    u8 m_prevKcpId; ///< @unused
    MapdataCheckPoint *m_prevPoints[6];
    LinkedCheckpoint m_nextPoints[6];
};

class MapdataCheckPointAccessor
    : public MapdataAccessorBase<MapdataCheckPoint, MapdataCheckPoint::SData> {
public:
    MapdataCheckPointAccessor(const MapSectionHeader *header);
    ~MapdataCheckPointAccessor() override;

    [[nodiscard]] s8 lastKcpType() const;

private:
    [[nodiscard]] f32 calculateMeanTotalDistanceRecursive(u16 ckptId);
    [[nodiscard]] f32 calculateMeanTotalDistance();
    void findFinishAndLastKcp();
    void init();
    s8 m_lastKcpType;
    u16 m_finishLineCheckpointId;
    f32 m_meanTotalDistance; ///< @unused
};

} // namespace System
