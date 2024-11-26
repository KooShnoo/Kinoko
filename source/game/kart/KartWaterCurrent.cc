#include <Logger.hh>
#include <cmath>
#include <egg/math/Math.hh>
#include <egg/math/ProjThing.hh>
#include <egg/math/Vector.hh>
#include <game/kart/KartWaterCurrent.hh>
#include <game/system/CourseMap.hh>
#include <game/system/RaceManager.hh>

namespace Kart {

KartWaterCurrent::KartWaterCurrent() {
    m_variantZero = new KartWaterCurrentVariantZero(this);
}

/// @addr{0x80593E18}
void KartWaterCurrent::updatePoint() {
    if (m_lastCalcdPoint == m_currentPoint) {
        return;
    }

    auto currPt = m_route->get(m_currentPoint);
    m_routeWaterCurrentStrength = currPt.setting1;
    auto nextPt = m_route->get(m_currentPoint + 1);
    m_flowDir = nextPt.pos - currPt.pos;
    m_flowDir.normalise();

    if (currPt.setting2 == 1) {
        m_flowDir = m_flowDir.surfNormal(EGG::Vector3f::ey).second;
        m_flowDir *= -1;
    } else if (currPt.setting2 == 2) {
        m_flowDir = m_flowDir.surfNormal(EGG::Vector3f::ey).second;
    }
    m_lastCalcdPoint = m_currentPoint;
}

/// @addr{0x80593DBC}
void KartWaterCurrent::onNewPoint(f64 unk, u16 newPt) {
    if ((m_weirdFloat >= 0 || unk >= 3000.0f) && m_weirdFloat <= unk) {
        return;
    }
    m_currentPoint = newPt;
    m_weirdFloat = unk;
    // m_variantOne->m_currentPt = newPt;
    updatePoint();
}

/// @addr{0x80594134}
void KartWaterCurrent::calc() {
    if (calcArea() && m_route->count() > 2) {
        m_variantZero->vf0c();
        // m_variantOne->vf0c();
    }
}

/// @addr{0x805941BC}
/// @brief checks the currently occupied AREA and updates the water current details appropriately
/// @returns whether the player is in an AREA associated with a water current.
bool KartWaterCurrent::calcArea() {
    // @todo
    if (System::RaceManager::Instance()->m_timer < 1252) {
        return false;
    }
    m_route = System::CourseMap::Instance()->getPointInfo(0);
    m_variantZero->m_route = m_route;
    return true;
}


/// @addr{0x8059345C}
bool KartWaterCurrentVariantZero::doWeirdPointMath(s32 ptOffset, s16 &outNewPt,
        EGG::Vector3f &outVec1, EGG::Vector3f &outVec2) {
    s32 ptOfsMinus1 = -1;
    s32 ptOfs0 = 0;
    s32 ptOfsPlus1 = 1;
    bool result = false;

    s16 currentPtr = m_currentPt;
    outNewPt = currentPtr;
    if (ptOffset == 1) {
        outNewPt = currentPtr + 1;
        ptOfsMinus1 = 0;
        ptOfs0 = 1;
        ptOfsPlus1 = 2;
    } else if (ptOffset == 2) {
        outNewPt = currentPtr - 1;
        ptOfsMinus1 = -2;
        ptOfs0 = -1;
        ptOfsPlus1 = 0;
    }

    bool off0Works = false;
    s32 idx_pt0 = m_currentPt + ptOfs0;

    if ((idx_pt0 >= 0) && m_route->isIdxValid(idx_pt0)) {
        off0Works = true;
    }

    if (off0Works) {
        s32 idx_pt1 = currentPtr + ptOfsPlus1;
        bool off1Works = false;

        if (idx_pt1 >= 0) {
            if (m_route->isIdxValid(idx_pt1)) {
                off1Works = true;
            }

            if (off1Works) {
                EGG::Vector3f off1Pos;
                EGG::Vector3f off2Pos;
                EGG::Vector3f offM1Pos;

                u16 newPt1 = idx_pt1;
                if (m_route->isIdxValidU(newPt1)) {
                    off1Pos = m_route->get(newPt1).pos;
                }

                u16 newPt = m_currentPt + ptOfsPlus1;
                if (m_route->isIdxValidU(newPt)) {
                    off2Pos = m_route->get(newPt).pos;
                }

                EGG::Vector3f routeDirRaw = (off1Pos - off2Pos);
                routeDirRaw.normalise();
                bool offM1Works = false;
                EGG::Vector3f routeDir = -routeDirRaw;

                s32 idx_ptMinus1 = m_currentPt + ptOfsMinus1;
                if (idx_ptMinus1 >= 0 && m_route->isIdxValid(idx_ptMinus1)) {
                    offM1Works = true;
                }
                if (offM1Works) {
                    u16 ptCheck = idx_ptMinus1;

                    if (m_route->isIdxValidU(ptCheck)) {
                        offM1Pos = m_route->get(ptCheck).pos;
                    }

                    (offM1Pos - off1Pos).normalise();
                }

                EGG::ProjThing proj1 = EGG::ProjThing(off1Pos, routeDirRaw);
                EGG::ProjThing proj2 = EGG::ProjThing(off2Pos, routeDir);

                if (proj1.isNegative(pos()) && proj2.isNegative(pos())) {
                    result = true;
                    // EGG::Vector3f meow = pos() - off1Pos;
                    outVec1 = off1Pos;
                    outVec2 = routeDir;
                }
            }
        }
    }

    return result;
}
KartWaterCurrentVariantZero::KartWaterCurrentVariantZero(KartWaterCurrent *waterCurrent) {
    m_currentPt = 0;
    m_route = nullptr;
    m_waterCurrent = waterCurrent;
}

/// @addr {0x80593138}
void KartWaterCurrentVariantZero::vf0c() {
    EGG::Vector3f local3c, local48;
    s16 newPt;
    if (doWeirdPointMath(0, newPt, local3c, local48)) {
        REPORT("newpt");
        EGG::Vector3f foo = pos() - local3c;
        auto dVar4 = foo.length();
        auto fVar1 = EGG::Mathf::abs(foo.dot(local48));
        auto dvar3 = EGG::Mathf::sqrt(dVar4 * dVar4 - fVar1 * fVar1);
        m_waterCurrent->onNewPoint(dvar3, newPt);
    }
    // sus
    REPORT("inc pt, %d", m_currentPt);
    m_currentPt++;
    if (m_currentPt >= m_route->count() - 1) {
        // if (m_poti->setting2() == 1) {}
        m_currentPt = 0;
    }
}

} // namespace Kart
