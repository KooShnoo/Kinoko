#pragma once

#include <egg/math/Vector.hh>
#include <game/kart/KartObjectProxy.hh>
#include <game/system/map/MapdataPointInfo.hh>

namespace Kart {

class KartWaterCurrentVariantZero;

class KartWaterCurrent : KartObjectProxy {
public:
    // née ks unk touches field 7, field 7 is `m_routeWaterCurrentStrength`
    void calcIpml();
    // vf10
    void calc(f64 unk, u16 newPt);

    [[nodiscard]] const EGG::Vector3f &flowDir() const {
        return m_flowDir;
    }

private:
    u32 m_routeId = -1;
    f32 m_weirdFloat = -1.0f;
    System::MapdataPointInfo *m_route = nullptr;
    u16 m_currentPoint = -1;
    u16 m_lastCalcdPoint = -1;
    EGG::Vector3f m_flowDir = EGG::Vector3f::zero;
    f32 m_routeWaterCurrentStrength = 0.0f;
    f32 m_parallelWaterCurrentStrength = 0.0f;
    const u8 m_count = 2; ///< this is always two; there are two types of water currents in mario
                          ///< kart wii, route-based and parallel(two-point route)

    // i think this might be the route one? idk maybe its the parallel one. maybe the variants are
    // actually sometihng else entirely???
    KartWaterCurrentVariantZero *m_variantZero;

    f32 m_area1SpeedFactor = 1.0f;
    u32 m_areaId = -1;
};

class KartWaterCurrentVariant : public KartObjectProxy {
public:
    virtual void vf0c() = 0;
};

class KartWaterCurrentVariantZero : KartWaterCurrentVariant {
public:
    virtual void vf0c() override;
    bool doWeirdPointMath(s32 ptOffset, s16 &outNewPt, EGG::Vector3f &outVec1,
            EGG::Vector3f &outVec2);

private:
    s16 m_currentPt;
    System::MapdataPointInfo *m_poti;
    KartWaterCurrent m_waterCurrent;
};

} // namespace Kart
