#pragma once

#include "game/kart/KartObject.hh"

namespace Field {

class JugemMove {
public:
    JugemMove(const Kart::KartObject *kartObj);
    ~JugemMove();

    void init();
    void calc();

    /// @addr{0x8071EFA8}
    void FUN_8071EFA8(const EGG::Vector3f &v0, bool set10) {
        m_pos = v0;

        if (set10) {
            m_10 = v0;
        }
    }

    void setForwardFromKartObjPosDelta(bool set124);
    void setForwardFromKartObjMainRot(bool set124);

    /// @addr{0x8071EFD8}
    void set_1c(const EGG::Vector3f &v) {
        m_1c = v;
    }

    void setRiseVel(const EGG::Vector3f &v) {
        m_riseVel = v;
    }

    void set_140(bool isSet) {
        m_140 = isSet;
    }

    void set_148(bool isSet) {
        m_148 = isSet;
    }

    void set_150(bool isSet) {
        m_150 = isSet;
    }

    const EGG::Vector3f &_10() const {
        return m_10;
    }

    const EGG::Matrix34f &transform() const {
        return m_transform;
    }

private:
    EGG::Matrix34f calcOrthonormalBasis();
    EGG::Vector3f calcOscillation(const EGG::Matrix34f &mat);
    [[nodiscard]] EGG::Matrix34f FUN_807202BC();

    [[nodiscard]] static EGG::Vector3f Interpolate(f32 t, const EGG::Vector3f &v0,
            const EGG::Vector3f &v1) {
        return v0 + (v1 - v0) * t;
    }

    const Kart::KartObject *m_kartObj;
    EGG::Vector3f m_pos;
    EGG::Vector3f m_10;
    EGG::Vector3f m_1c;
    EGG::Matrix34f m_28;
    EGG::Matrix34f m_58;
    EGG::Matrix34f m_88;
    EGG::Matrix34f m_transform;
    f32 m_phaseX; ///< Left/right oscillation phase
    f32 m_phaseY; ///< Up/down oscillation phase
    EGG::Vector3f m_f4;
    EGG::Vector3f m_velDir;
    EGG::Vector3f m_riseVel; ///< Velocity of Lakitu once he leaves by rising upwards
    EGG::Vector3f m_118;
    EGG::Vector3f m_currForward;   ///< The current direction that Lakitu is at
    EGG::Vector3f m_targetForward; ///< The direction that Lakitu wants to move to
    f32 m_forwardInterpRate;
    bool m_140;
    f32 m_144;
    bool m_148;
    bool m_150;
};

} // namespace Field
