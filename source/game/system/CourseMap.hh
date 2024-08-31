#pragma once

#include <egg/math/Vector.hh>

/// @brief High-level handling for generic system operations, such as input reading, race
/// configuration, and resource management.
namespace System {

class MapdataCannonPoint;
class MapdataCannonPointAccessor;
class MapdataCheckPath;
class MapdataCheckPathAccessor;
class MapdataCheckPoint;
class MapdataCheckPointAccessor;
class MapdataFileAccessor;
class MapdataGeoObj;
class MapdataGeoObjAccessor;
class MapdataJugemPoint;
class MapdataJugemPointAccessor;
class MapdataStageInfo;
class MapdataStageInfoAccessor;
class MapdataStartPoint;
class MapdataStartPointAccessor;

/// @brief Contains course metadata, notably the starting position.
/// @addr{0x809BD6E8}
/// @nosubgrouping
class CourseMap {
public:
    void init();
    [[nodiscard]] MapdataCannonPointAccessor *parseCannonPoint(u32 sectionName);
    [[nodiscard]] MapdataCheckPointAccessor *parseCheckPoint(u32 sectionName);
    [[nodiscard]] MapdataCheckPathAccessor *parseCheckPath(u32 sectionName);
    [[nodiscard]] MapdataGeoObjAccessor *parseGeoObj(u32 sectionName);
    [[nodiscard]] MapdataJugemPointAccessor *parseJugemPoint(u32 sectionName);
    [[nodiscard]] MapdataStageInfoAccessor *parseStageInfo(u32 sectionName);
    [[nodiscard]] MapdataStartPointAccessor *parseStartPoint(u32 sectionName);

    /// @beginGetters
    [[nodiscard]] MapdataCannonPoint *getCannonPoint(u16 i) const;
    [[nodiscard]] MapdataCheckPoint *getCheckPoint(u16 i) const;
    [[nodiscard]] MapdataCheckPath *getCheckPath(u16 i) const;
    [[nodiscard]] MapdataCheckPathAccessor *checkPath() const;
    [[nodiscard]] MapdataGeoObj *getGeoObj(u16 i) const;
    [[nodiscard]] MapdataJugemPoint *getJugemPoint(u16 i) const;
    [[nodiscard]] MapdataStageInfo *getStageInfo() const;
    [[nodiscard]] MapdataStartPoint *getStartPoint(u16 i) const;
    [[nodiscard]] u16 getGeoObjCount() const;
    [[nodiscard]] u16 getJugemPointCount() const;
    [[nodiscard]] s8 lastKcpType() const;
    [[nodiscard]] u32 version() const;
    [[nodiscard]] f32 startTmpAngle() const;
    [[nodiscard]] f32 startTmp0() const;
    [[nodiscard]] f32 startTmp1() const;
    [[nodiscard]] f32 startTmp2() const;
    [[nodiscard]] f32 startTmp3() const;
    /// @endGetters

    [[nodiscard]] u16 getCheckPointCount() const;
    [[nodiscard]] u16 getCheckPathCount() const;
    void clearSectorChecked();
    [[nodiscard]] s16 findSector(s32 playerIdx, const EGG::Vector3f &pos, u16 checkpointIdx, f32 *checkpointCompletion,
            bool isRemote);
    [[nodiscard]] s16 searchNextCheckpoint(s32 playerIdx, const EGG::Vector3f &pos, s16 depth,
            const MapdataCheckPoint &checkpoint, float *completion, u32 params,
            const bool param_8) const;
    [[nodiscard]] s16 searchPrevCheckpoint(s32 playerIdx, const EGG::Vector3f &pos, s16 depth,
            const MapdataCheckPoint &checkpoint, float *completion, u32 params,
            const bool param_8) const;
    [[nodiscard]] s16 findRecursiveSector(s32 playerIdx, const EGG::Vector3f &pos, s16 depth, int param_5,
            MapdataCheckPoint &checkpoint, float *completion, u32 params) const;

    static CourseMap *CreateInstance();
    static void DestroyInstance();
    [[nodiscard]] static CourseMap *Instance();

private:
    CourseMap();
    ~CourseMap();

    MapdataFileAccessor *m_course;
    MapdataStartPointAccessor *m_startPoint;
    MapdataGeoObjAccessor *m_geoObj;
    MapdataJugemPointAccessor *m_jugemPoint;
    MapdataCannonPointAccessor *m_cannonPoint;
    MapdataCheckPointAccessor *m_checkPoint;
    MapdataCheckPathAccessor *m_checkPath;
    MapdataStageInfoAccessor *m_stageInfo;

    // TODO: Better names
    f32 m_startTmpAngle;
    f32 m_startTmp0;
    f32 m_startTmp1;
    f32 m_startTmp2;
    f32 m_startTmp3;

    static void *LoadFile(const char *filename); ///< @addr{0x809BD6E8}

    static CourseMap *s_instance;
};

} // namespace System
