#pragma once

#include "game/field/jugem/JugemMove.hh"
#include "game/field/jugem/JugemSwitch.hh"

#include "game/field/StateManager.hh"

#include "game/kart/KartObject.hh"

namespace Field {

class JugemUnitSub210 {
public:
    JugemUnitSub210(u32 count) {
        m_arr = new idk[count];
        _08 = 0.0f;
        m_0c = 0.0f;
    }

    /// @addr{0x80721E64}
    ~JugemUnitSub210() {
        delete m_arr;
    }

    /// @addr{0x8072370C}
    void FUN_8072370C(f32 param1, f32 param2, f32 param3, u32 param4) {
        _04 = 0;

        FUN_8074C048(param1);
        FUN_8074C0B4(param2, param3, param4);

        m_0c = param1;
        _08 = 0.0f;
    }

    /// @addr{0x80723AF8}
    bool FUN_80723AF8(f32 param1) {
        m_0c = FUN_8074C1E0(_08);
        _08 += param1;

        bool bVar3 = _08 > m_arr[1]._04;
        if (bVar3) {
            _08 = m_arr[1]._04;
        }

        return bVar3;
    }

    /// @addr{0x8074C048}
    void FUN_8074C048(f32 param1) {
        m_arr[_04]._00 = param1;
        m_arr[_04]._04 = 0.0f;
        ++_04;
    }

    /// @addr{0x8074C0B4}
    void FUN_8074C0B4(f32 param1, f32 param2, u32 /*param4*/) {
        m_arr[_04]._00 = param1;
        m_arr[_04]._04 = param2 + m_arr[_04 - 1]._04;
        ++_04;
    }

    /// @addr{0x8074C1E0}
    f32 FUN_8074C1E0(f32 param1) {
        idk *ptr = nullptr;
        idk *nextPtr = nullptr;
        u32 iVar5 = 0;

        u32 iVar1 = _04;
        while (--iVar1 != 0) {
            ptr = &m_arr[iVar5];
            nextPtr = &m_arr[iVar5 + 1];
            if (ptr->_04 <= param1 && param1 < nextPtr->_04) {
                break;
            }
        }

        f32 f2 = ptr->_04;
        f32 f0 = nextPtr->_04;
        f32 f1 = param1 - f2;
        f2 = f0 - f2;

        f1 = std::clamp(f1, 0.0f, f2);

        return Lerp(ptr->_00, nextPtr->_00, f1 / f2);
    }

    [[nodiscard]] f32 _0c() const {
        return m_0c;
    }

private:
    struct idk {
        idk() : _00(0.0f), _04(0.0f) {}

        f32 _00;
        f32 _04;
    };

    /// @addr{0x8074C3F0}
    [[nodiscard]] static f32 Lerp(f32 param1, f32 param2, f32 t) {
        f32 dVar4 = EGG::Mathf::SinFIdx(RAD2FIDX * (-HALF_PI + t * (HALF_PI - -HALF_PI)));
        f32 fVar1 = 0.5f * (1.0f + dVar4);
        f32 fVar2 = std::clamp(fVar1, 0.0f, 1.0f);

        return param1 + fVar2 * (param2 - param1);
    }

    idk *m_arr;
    u32 _04;  ///< Some index
    f32 _08;  ///< Some timer that controls descent
    f32 m_0c; ///< Some interpolation t
};

class JugemUnit : public StateManager {
public:
    JugemUnit(const Kart::KartObject *kartObj);
    ~JugemUnit();

    /// @addr{0x80721EC0}
    void createSwitchRace() {
        m_switchReverse = new JugemSwitchReverse;
    }

    /// @addr{0x80722100}
    void init() {
        m_pos.setZero();
        m_move->init();
    }

    void calc();

private:
    enum class State {
        Away = 0,
        Descending = 1,
        Stay = 2,
        Ascending = 3,
    };

    /// @addr{0x80723458}
    void enterIdle() {
        m_state = State::Away;
        m_ascendTimer = 0;
    }

    void enterReverse();

    /// @addr{0x807234A4}
    void calcIdle() {
        if (m_switchReverse && m_switchReverse->isOn()) {
            m_nextStateId = 1;
        }
    }

    void calcReverse();

    /// @addr{0x80722ED8}
    void calcSwitches() {
        if (m_switchReverse) {
            m_switchReverse->calc();
        }
    }

    /// @addr{0x80722F6C}
    void setPosFromTransform(const EGG::Matrix34f &mat) {
        m_pos = mat.base(3);
        m_pos.y = m_kartObj->pos().y;
    }

    EGG::Vector3f FUN_807230D4(const EGG::Vector3f &v);
    void calcCollision();

    const Kart::KartObject *m_kartObj;
    EGG::Vector3f m_pos;
    State m_state;
    u32 m_ascendTimer; ///< How long Lakitu has been disappearing/ascending for
    JugemSwitch *m_switchReverse;
    JugemMove *m_move;
    JugemUnitSub210 *m_210;

    [[nodiscard]] static EGG::Vector3f Interpolate(f32 t, const EGG::Vector3f &v0,
            const EGG::Vector3f &v1) {
        return v0 + (v1 - v0) * t;
    }

    static constexpr std::array<StateManagerEntry, 2> STATE_ENTRIES = {{
            StateEntry<JugemUnit, &JugemUnit::enterIdle, &JugemUnit::calcIdle>(0),
            StateEntry<JugemUnit, &JugemUnit::enterReverse, &JugemUnit::calcReverse>(1),
    }};
};

} // namespace Field
