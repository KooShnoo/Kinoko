#pragma once

#include "game/field/CourseColMgr.hh"

#include <egg/math/Vector.hh>

#define COLLISION_ARR_LENGTH 0x40

/// @brief Pertains to collision.
namespace Field {

/// @brief Manages the caching of colliding KCL triangles and exposes queries for collision checks.
/// @addr{0x809C2F44}
/// @nosubgrouping
class CollisionDirector : EGG::Disposer {
public:
    struct CollisionEntry {
        KCLTypeMask typeMask;
        u16 attribute;
        f32 dist;
    };

    void checkCourseColNarrScLocal(f32 radius, const EGG::Vector3f &pos, KCLTypeMask mask,
            u32 /*unused*/);

    [[nodiscard]] bool checkSphereFull(f32 radius, const EGG::Vector3f &v0, const EGG::Vector3f &v1,
            KCLTypeMask flags, CourseColMgr::CollisionInfo *pInfo, KCLTypeMask *pFlagsOut,
            u32 /*start*/);
    [[nodiscard]] bool checkSphereFullPush(f32 radius, const EGG::Vector3f &v0,
            const EGG::Vector3f &v1, KCLTypeMask flags, CourseColMgr::CollisionInfo *pInfo,
            KCLTypeMask *pFlagsOut, u32 /*start*/);

    [[nodiscard]] bool checkSphereCachedPartial(const EGG::Vector3f &pos,
            const EGG::Vector3f &prevPos, KCLTypeMask typeMask,
            CourseColMgr::CollisionInfo *colInfo, KCLTypeMask *typeMaskOut, f32 radius);
    [[nodiscard]] bool checkSphereCachedPartialPush(const EGG::Vector3f &pos,
            const EGG::Vector3f &prevPos, KCLTypeMask typeMask,
            CourseColMgr::CollisionInfo *colInfo, KCLTypeMask *typeMaskOut, f32 radius, u32 start);
    [[nodiscard]] bool checkSphereCachedFullPush(const EGG::Vector3f &pos,
            const EGG::Vector3f &prevPos, KCLTypeMask typeMask,
            CourseColMgr::CollisionInfo *colInfo, KCLTypeMask *typeMaskOut, f32 radius, u32 start);

    void resetCollisionEntries(KCLTypeMask *ptr);
    void pushCollisionEntry(f32 dist, KCLTypeMask *typeMask, KCLTypeMask kclTypeBit, u16 attribute);

    bool findClosestCollisionEntry(KCLTypeMask *typeMask, KCLTypeMask type);

    /// @beginGetters
    [[nodiscard]] const CollisionEntry *closestCollisionEntry() const {
        return m_closestCollisionEntry;
    }
    /// @endGetters

    static CollisionDirector *CreateInstance();
    static void DestroyInstance();

    [[nodiscard]] static CollisionDirector *Instance() {
        return s_instance;
    }

private:
    CollisionDirector();
    ~CollisionDirector() override;

    const CollisionEntry *m_closestCollisionEntry;
    std::array<CollisionEntry, COLLISION_ARR_LENGTH> m_entries;
    size_t m_collisionEntryCount;

    static CollisionDirector *s_instance; ///< @addr{0x809C2F44}
};

} // namespace Field
