#include "CourseMap.hh"

#include "game/system/map/MapdataCannonPoint.hh"
#include "game/system/map/MapdataCheckPoint.hh"
#include "game/system/map/MapdataFileAccessor.hh"
#include "game/system/map/MapdataGeoObj.hh"
#include "game/system/map/MapdataJugemPoint.hh"
#include "game/system/map/MapdataStageInfo.hh"
#include "game/system/map/MapdataStartPoint.hh"

#include "game/system/ResourceManager.hh"

namespace System {

/// @addr{0x805127EC}
void CourseMap::init() {
    void *buffer = LoadFile("course.kmp");
    m_course =
            new MapdataFileAccessor(reinterpret_cast<const MapdataFileAccessor::SData *>(buffer));

    constexpr u32 CANNON_POINT_SIGNATURE = 0x434e5054;
    constexpr u32 CHECK_POINT_SIGNATURE = 0x434b5054;
    constexpr u32 GEO_OBJ_SIGNATURE = 0x474f424a;
    constexpr u32 JUGEM_POINT_SIGNATURE = 0x4a475054;
    constexpr u32 START_POINT_SIGNATURE = 0x4b545054;
    constexpr u32 STAGE_INFO_SIGNATURE = 0x53544749;

    m_startPoint = parseStartPoint(START_POINT_SIGNATURE);
    m_geoObj = parseGeoObj(GEO_OBJ_SIGNATURE);
    m_jugemPoint = parseJugemPoint(JUGEM_POINT_SIGNATURE);
    m_cannonPoint = parseCannonPoint(CANNON_POINT_SIGNATURE);
    m_checkPoint = parseCheckPoint(CHECK_POINT_SIGNATURE);
    m_stageInfo = parseStageInfo(STAGE_INFO_SIGNATURE);

    MapdataStageInfo *stageInfo = getStageInfo();
    constexpr u8 TRANSLATION_MODE_NARROW = 1;
    if (stageInfo && stageInfo->translationMode() == TRANSLATION_MODE_NARROW) {
        m_startTmpAngle = 25.0f;
        m_startTmp2 = 250.0f;
        m_startTmp3 = 0.0f;
    } else {
        m_startTmpAngle = 30.0f;
        m_startTmp2 = 400.0f;
        m_startTmp3 = 100.0f;
    }

    m_startTmp0 = 800.0f;
    m_startTmp1 = 1200.0f;
}

/// @addr{0x80512FA4}
MapdataCannonPointAccessor *CourseMap::parseCannonPoint(u32 sectionName) {
    const MapSectionHeader *sectionPtr = m_course->findSection(sectionName);

    MapdataCannonPointAccessor *accessor = nullptr;
    if (sectionPtr) {
        accessor = new MapdataCannonPointAccessor(sectionPtr);
    }

    return accessor;
}

/// @addr{0x80513640}
MapdataCheckPointAccessor *CourseMap::parseCheckPoint(u32 sectionName) {
    const MapSectionHeader *sectionPtr = m_course->findSection(sectionName);

    MapdataCheckPointAccessor *accessor = nullptr;
    if (sectionPtr) {
        accessor = new MapdataCheckPointAccessor(sectionPtr);
    }

    return accessor;
}

/// @addr{0x805134C8}
MapdataGeoObjAccessor *CourseMap::parseGeoObj(u32 sectionName) {
    const MapSectionHeader *sectionPtr = m_course->findSection(sectionName);

    MapdataGeoObjAccessor *accessor = nullptr;
    if (sectionPtr) {
        accessor = new MapdataGeoObjAccessor(sectionPtr);
    }

    return accessor;
}

/// @addr{0x805130C4}
MapdataJugemPointAccessor *CourseMap::parseJugemPoint(u32 sectionName) {
    const MapSectionHeader *sectionPtr = m_course->findSection(sectionName);

    return sectionPtr ? new MapdataJugemPointAccessor(sectionPtr) : nullptr;
}

/// @addr{0x80512D64}
MapdataStageInfoAccessor *CourseMap::parseStageInfo(u32 sectionName) {
    const MapSectionHeader *sectionPtr = m_course->findSection(sectionName);

    MapdataStageInfoAccessor *accessor = nullptr;
    if (sectionPtr) {
        accessor = new MapdataStageInfoAccessor(sectionPtr);
    }

    return accessor;
}

/// @addr{0x80513F5C}
MapdataStartPointAccessor *CourseMap::parseStartPoint(u32 sectionName) {
    const MapSectionHeader *sectionPtr = m_course->findSection(sectionName);

    MapdataStartPointAccessor *accessor = nullptr;
    if (sectionPtr) {
        accessor = new MapdataStartPointAccessor(sectionPtr);
    }

    return accessor;
}

/// @addr{0x80518AE0}
MapdataCannonPoint *CourseMap::getCannonPoint(u16 i) const {
    return m_cannonPoint && m_cannonPoint->size() != 0 ? m_cannonPoint->get(i) : nullptr;
}

/// @addr{0x80518AE0}
MapdataCheckPoint *CourseMap::getCheckPoint(u16 i) const {
    return m_checkPoint && m_checkPoint->size() != 0 ? m_checkPoint->get(i) : nullptr;
}

/// @addr{0x80514148}
MapdataGeoObj *CourseMap::getGeoObj(u16 i) const {
    return i < getGeoObjCount() ? m_geoObj->get(i) : nullptr;
}

/// @addr{0x80518920}
MapdataJugemPoint *CourseMap::getJugemPoint(u16 i) const {
    return i < getJugemPointCount() ? m_jugemPoint->get(i) : nullptr;
}

/// @addr{0x80518B78}
MapdataStageInfo *CourseMap::getStageInfo() const {
    return m_stageInfo && m_stageInfo->size() != 0 ? m_stageInfo->get(0) : nullptr;
}

/// @addr{0x80514B30}
MapdataStartPoint *CourseMap::getStartPoint(u16 i) const {
    return m_startPoint && m_startPoint->size() != 0 ? m_startPoint->get(i) : nullptr;
}

u16 CourseMap::getGeoObjCount() const {
    return m_geoObj ? m_geoObj->size() : 0;
}

u16 CourseMap::getJugemPointCount() const {
    return m_jugemPoint ? m_jugemPoint->size() : 0;
}

u32 CourseMap::version() const {
    return m_course->version();
}

f32 CourseMap::startTmpAngle() const {
    return m_startTmpAngle;
}

f32 CourseMap::startTmp0() const {
    return m_startTmp0;
}

f32 CourseMap::startTmp1() const {
    return m_startTmp1;
}

f32 CourseMap::startTmp2() const {
    return m_startTmp2;
}

f32 CourseMap::startTmp3() const {
    return m_startTmp3;
}

u16 CourseMap::getCheckPointCount() const {
    return m_checkPoint ? m_checkPoint->size() : 0;
}

void CourseMap::clearSectorChecked() {
    for (u16 i = 0; i < getCheckPointCount(); i++) {
        getCheckPoint(i)->resetFlags();
    }
}

// This is a disaster
/// @addr{0x80511500}
s16 CourseMap::findSector(s32 playerIdx, const EGG::Vector3f& pos, u16 checkpointIdx, f32* distanceRatio, bool isRemote) {
    clearSectorChecked();
    MapdataCheckPoint *checkpoint = getCheckPoint(checkpointIdx);
    s16 id = -1;
    MapdataCheckPoint::Completion completion = checkpoint->checkSectorAndDistanceRatio(pos, distanceRatio);
    checkpoint->setPlayerFlags(playerIdx);
    u32 params = 0;
    if (isRemote)
        params = 6;

    switch (completion) {
    case MapdataCheckPoint::Completion_0:
        id = checkpoint->id();
        break;
    case MapdataCheckPoint::Completion_2:
        if (*distanceRatio > 0.5f) {
            // Search next 
            for (s32 i = 0; i < checkpoint->nextCount(); i++) {
                MapdataCheckPoint *checkpoint_ = i < checkpoint->nextCount() ? checkpoint->nextPoint(i) : nullptr;
                id = findRecursiveSector(playerIdx, pos, 1, 0, *checkpoint_, distanceRatio, params);
                if (id != -1)
                    break;
            }

            // If that fails, search next -> prev
            if (id == -1) {
                for (s32 i = 0; i < checkpoint->nextCount(); i++) {
                    MapdataCheckPoint *next = i < checkpoint->nextCount() ? checkpoint->nextPoint(i) : nullptr;
                    for (s32 j = 0; j < next->prevCount(); j++) {
                        MapdataCheckPoint *prev = j < next->prevCount() ? next->prevPoint(j) : nullptr;
                        if (prev == checkpoint)
                            continue;

                        id = findRecursiveSector(playerIdx, pos, 1, 1, *prev, distanceRatio, params);
                        if (id != -1)
                            break;
                    }
                }
                // If that fails, search prev -> next
                if (id == -1) {
                    for (s32 i = 0; i < checkpoint->prevCount(); i++) {
                        MapdataCheckPoint *prev = (s32)i < checkpoint->prevCount() ? checkpoint->prevPoint(i) : nullptr;
                        for (s32 j = 0; j < prev->nextCount(); j++) {
                            MapdataCheckPoint *next = (s32)j < prev->nextCount() ? prev->nextPoint(j) : nullptr;
                            if (next == checkpoint)
                                continue;

                            id = findRecursiveSector(playerIdx, pos, 1, 0, *next, distanceRatio, params);
                            if (id != -1)
                                break;
                        }
                    }
                }
            }
            // If that fails, search prev
            if (id == -1) {
                for (s32 i = 0; i < checkpoint->prevCount(); i++) {
                    MapdataCheckPoint *checkpoint_ = i < checkpoint->prevCount() ? checkpoint->prevPoint(i) : nullptr;
                    id = findRecursiveSector(playerIdx, pos, 1, 1, *checkpoint_, distanceRatio, params);
                    if (id != -1)
                        break;
                }
            }
        } else {
            // Search prev
            for (s32 i = 0; i < checkpoint->prevCount(); i++) {
                MapdataCheckPoint *checkpoint_ = i < checkpoint->prevCount() ? checkpoint->prevPoint(i) : nullptr;
                id = findRecursiveSector(playerIdx, pos, 1, 1, *checkpoint_, distanceRatio, params);
                if (id != -1)
                    break;
            }
            // If that fails, search prev -> next
            if (id == -1) {
                for (s32 i = 0; i < checkpoint->prevCount(); i++) {
                    MapdataCheckPoint *prev = (s32)i < checkpoint->prevCount() ? checkpoint->prevPoint(i) : nullptr;
                    for (s32 j = 0; j < prev->nextCount(); j++) {
                        MapdataCheckPoint *next = (s32)j < prev->nextCount() ? prev->nextPoint(j) : nullptr;
                        if (next == checkpoint)
                            continue;

                        id = findRecursiveSector(playerIdx, pos, 1, 0, *next, distanceRatio, params);
                        if (id != -1)
                            break;
                    }
                }

                // If that fails, search next -> prev
                if (id == -1) {
                    for (s32 i = 0; i < checkpoint->nextCount(); i++) {
                        MapdataCheckPoint *next = (s32)i < checkpoint->nextCount() ? checkpoint->nextPoint(i) : nullptr;
                        for (s32 j = 0; j < next->prevCount(); j++) {
                            MapdataCheckPoint *prev = (s32)j < next->prevCount() ? next->prevPoint(j) : nullptr;
                            if (prev == checkpoint)
                                continue;

                            id = findRecursiveSector(playerIdx, pos, 1, 1, *prev, distanceRatio, params);
                            if (id != -1)
                                break;
                        }
                    }
                }
            }
            // If that fails, search next
            if (id == -1) {
                for (s32 i = 0; i < checkpoint->nextCount(); i++) {
                    MapdataCheckPoint *checkpoint_ = i < checkpoint->nextCount() ? checkpoint->nextPoint(i) : nullptr;
                    id = findRecursiveSector(playerIdx, pos, 1, 0, *checkpoint_, distanceRatio, params);
                    if (id != -1)
                        break;
                }
            }
        }
        break;
    case MapdataCheckPoint::Completion_1:
        // Search next -> prev
        for (s32 i = 0; i < checkpoint->nextCount(); i++) {
            MapdataCheckPoint *next = (s32)i < checkpoint->nextCount() ? checkpoint->nextPoint(i) : nullptr;
            for (s32 j = 0; j < next->prevCount(); j++) {
                MapdataCheckPoint *prev = (s32)j < next->prevCount() ? next->prevPoint(j) : nullptr;
                if (prev == checkpoint)
                    continue;

                id = findRecursiveSector(playerIdx, pos, 1, 1, *prev, distanceRatio, params);
                if (id != -1)
                    break;
            }
        }

        if (id == -1) {
            // If that fails, search prev -> next
            for (s32 i = 0; i < checkpoint->prevCount(); i++) {
                MapdataCheckPoint *prev = i < checkpoint->prevCount() ? checkpoint->prevPoint(i) : nullptr;
                for (s32 j = 0; j < prev->nextCount(); j++) {
                    MapdataCheckPoint *next = j < prev->nextCount() ? prev->nextPoint(j) : nullptr;
                    if (next == checkpoint)
                        continue;

                    id = findRecursiveSector(playerIdx, pos, 1, 0, *next, distanceRatio, params);
                    if (id != -1)
                        break;
                }
            }
        }

        if (id == -1) {
            // If that fails, search next
            for (s32 i = 0; i < checkpoint->nextCount(); i++) {
                MapdataCheckPoint *checkpoint_ = i < checkpoint->nextCount() ? checkpoint->nextPoint(i) : nullptr;
                id = findRecursiveSector(playerIdx, pos, 1, 0, *checkpoint_, distanceRatio, params);
                if (id != -1)
                    break;
            }
        }

        if (id == -1) {
            // If that fails, search prev
            for (s32 i = 0; i < checkpoint->prevCount(); i++) {
                MapdataCheckPoint *checkpoint_ = i < checkpoint->prevCount() ? checkpoint->prevPoint(i) : nullptr;
                id = findRecursiveSector(playerIdx, pos, 1, 1, *checkpoint_, distanceRatio, params);
                if (id != -1)
                    break;
            }
        }
        
    default:
        break;
    }

    // In the fallback case, search next
    if (id == -1) {
        for (s32 i = 0; i < checkpoint->nextCount(); i++) {
            MapdataCheckPoint *checkpoint_ = i < checkpoint->nextCount() ? checkpoint->nextPoint(i) : nullptr;
            id = findRecursiveSector(playerIdx, pos, -1, 0, *checkpoint_, distanceRatio, 0);
            if (id != -1)
                break;
        }
        // If that fails, search prev
        if (id == -1) {
            for (s32 i = 0; i < checkpoint->prevCount(); i++) {
                MapdataCheckPoint *checkpoint_ = i < checkpoint->prevCount() ? checkpoint->prevPoint(i) : nullptr;
                id = findRecursiveSector(playerIdx, pos, -1, 1, *checkpoint_, distanceRatio, 0);
                if (id != -1)
                    break;
            }
        }
    }

    if (isRemote && id == -1) {
        for (u16 i = 0; i < getCheckPointCount(); i++) {
            MapdataCheckPoint *checkpoint_ = getCheckPoint(i);
            if (!checkpoint_->isPlayerFlagged(playerIdx)) {
                MapdataCheckPoint::Completion completion = checkpoint_->checkSectorAndDistanceRatio(pos, distanceRatio);
                checkpoint_->setPlayerFlags(playerIdx);
                if (completion == MapdataCheckPoint::Completion_0) {
                    id = i;
                    break;
                }
            }
        }
    }

    return id;
}


s16 CourseMap::searchNextCheckpoint(s32 playerIdx, const EGG::Vector3f &pos, s16 depth, const MapdataCheckPoint &checkpoint, float *completion, u32 params, const bool param_8) const {
    s16 id = -1;
    s16 depth_ = depth >= 0 ? depth + 1 : -1;

    for (u16 i = 0; i < checkpoint.nextCount(); i++) {
        MapdataCheckPoint *checkpoint_ = (s32)i < checkpoint.nextCount() ? checkpoint.nextPoint(i) : nullptr;
        if (!param_8 || !checkpoint_->isPlayerFlagged(playerIdx)) {
            id = findRecursiveSector(playerIdx, pos, depth_, 0, *checkpoint_, completion, params);
            if (id != -1)
                break;
        }
    }
    return id;
}

s16 CourseMap::searchPrevCheckpoint(s32 playerIdx, const EGG::Vector3f &pos, s16 depth, const MapdataCheckPoint &checkpoint, float *completion, u32 params, const bool param_8) const {
    s16 id = -1;
    s16 depth_ = depth >= 0 ? depth + 1 : -1;

    for (u16 i = 0; i < checkpoint.prevCount(); i++) {
        MapdataCheckPoint *checkpoint_ = (s32)i < checkpoint.prevCount() ? checkpoint.prevPoint(i) : nullptr;
        if (!param_8 || !checkpoint_->isPlayerFlagged(playerIdx)) {
            id = findRecursiveSector(playerIdx, pos, depth_, 1, *checkpoint_, completion, params);
            if (id != -1)
                break;
        }
    }
    return id;
}

// TODO: param_5 is "eSearchType"
/// @addr{0x80511110}
s16 CourseMap::findRecursiveSector(s32 playerIdx, const EGG::Vector3f &pos, s16 depth, int param_5, MapdataCheckPoint &checkpoint, float *distanceRatio, u32 params) const {
    s16 maxDepth = params & 4 ? 12 : 6;
    if (depth >= 0 && depth > maxDepth) {
        return -1;
    }

    bool flagged = checkpoint.isPlayerFlagged(playerIdx);
    MapdataCheckPoint::Completion completion = MapdataCheckPoint::Completion_1;
    if (!flagged) {
        completion = checkpoint.checkSectorAndDistanceRatio(pos, distanceRatio);
    }
    checkpoint.setPlayerFlags(playerIdx);
    if (completion == MapdataCheckPoint::Completion_0)
        return checkpoint.id();

    if (param_5 == 0) {
        if (params & 1 && completion == MapdataCheckPoint::Completion_2 && *distanceRatio < 0.0f) {
            *distanceRatio = 0.0f;
            return checkpoint.id();
        }

        if (!(params & 2) && checkpoint.lapCheck() >= 0)
            return -1;

        u32 params_;
        if (completion == MapdataCheckPoint::Completion_2 && *distanceRatio > 0.0f)
            params_ = params | 1;
        else
            params_ = params &~ 1;
        
        s16 id = searchNextCheckpoint(playerIdx, pos, depth, checkpoint, distanceRatio, params_, false);
        return id == -1 ? searchPrevCheckpoint(playerIdx, pos, depth, checkpoint, distanceRatio, params_, true) : id;
    }

    if (params & 1 && completion == MapdataCheckPoint::Completion_2 && *distanceRatio > 1.0f) {
        *distanceRatio = 1.0f;
        return checkpoint.id();
    }

    if (!(params & 2) && checkpoint.lapCheck() >= 0)
        return -1;

    u32 params_;
    if (completion == MapdataCheckPoint::Completion_2 && *distanceRatio < 0.0f)
        params_ = params | 1;
    else
        params_ = params &~ 1;

    s16 id = searchPrevCheckpoint(playerIdx, pos, depth, checkpoint, distanceRatio, params_, false);
    return id == -1 ? searchNextCheckpoint(playerIdx, pos, depth, checkpoint, distanceRatio, params_, true) : id;
}

/// @addr{0x80512694}
CourseMap *CourseMap::CreateInstance() {
    assert(!s_instance);
    s_instance = new CourseMap;
    return s_instance;
}

/// @addr{0x8051271C}
void CourseMap::DestroyInstance() {
    assert(s_instance);
    delete s_instance;
    s_instance = nullptr;
}

CourseMap *CourseMap::Instance() {
    return s_instance;
}

/// @addr{0x8051276C}
CourseMap::CourseMap()
    : m_course(nullptr), m_startPoint(nullptr), m_stageInfo(nullptr), m_startTmpAngle(0.0f),
      m_startTmp0(0.0f), m_startTmp1(0.0f), m_startTmp2(0.0f), m_startTmp3(0.0f) {}

/// @addr{0x805127AC}
CourseMap::~CourseMap() {
    delete m_course;
    delete m_startPoint;
    delete m_stageInfo;
}

/// @addr{0x80512C10}
void *CourseMap::LoadFile(const char *filename) {
    return ResourceManager::Instance()->getFile(filename, nullptr, ArchiveId::Course);
}

CourseMap *CourseMap::s_instance = nullptr; ///< @addr{0x809BD6E8}

} // namespace System
