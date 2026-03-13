#include "JugemUnit.hh"

#include "game/field/CollisionDirector.hh"

#include "game/kart/KartMove.hh"
#include "game/kart/KartObjectManager.hh"

#include "game/system/RaceManager.hh"

namespace Field {

/// @addr{0x80721514}
JugemUnit::JugemUnit(const Kart::KartObject *kartObj)
    : StateManager(this, STATE_ENTRIES), m_kartObj(kartObj), m_switchReverse(nullptr) {
    m_move = new JugemMove(kartObj);
    m_210 = new JugemUnitSub210(2);
}

/// @addr{0x80721D2C}
JugemUnit::~JugemUnit() {
    delete m_switchReverse;
    delete m_move;
    delete m_210;
}

/// @addr{0x807221C4}
void JugemUnit::calc() {
    calcSwitches();

    StateManager::calc();

    // Perform a collision check if Lakitu is not idle
    if (m_currentStateId != 0) {
        setPosFromTransform(m_move->transform());
        calcCollision();
    }
}

/// @addr{0x80724794}
void JugemUnit::enterReverse() {
    constexpr EGG::Vector3f VEC3_809C2A64 = EGG::Vector3f(0.0f, 2000.0f, 0.0f);

    m_state = State::Descending;
    m_210->FUN_8072370C(0.0f, 1.0f, 40.0f, 3);
    m_move->init();
    EGG::Vector3f avStack_28 = FUN_807230D4(VEC3_809C2A64);
    m_move->FUN_8071EFA8(avStack_28, true);
    m_move->setForwardFromKartObjPosDelta(true);
    m_move->set_148(false);
}

/// @addr{0x80724880}
void JugemUnit::calcReverse() {
    constexpr EGG::Vector3f VEC_809C2A64 = EGG::Vector3f(0.0f, 2000.0f, 0.0f);
    constexpr EGG::Vector3f VEC_809C2A7C = EGG::Vector3f(0.0f, 500.0f, 0.0f);
    constexpr EGG::Vector3f SLOW_RISE_VEL = EGG::Vector3f(0.0f, 30.0f, 0.0f);
    constexpr EGG::Vector3f FAST_RISE_VEL = EGG::Vector3f(0.0f, 80.0f, 0.0f);
    constexpr EGG::Vector3f VEC_809C2A94 = EGG::Vector3f(0.0f, 250.0f, 350.0f);

    EGG::Vector3f local_40 = m_kartObj->pos() - m_move->_10();

    switch (m_state) {
    case State::Away: {
        bool iVar2 = m_210->FUN_80723AF8(1.0f);
        EGG::Vector3f local_4c = Interpolate(m_210->_0c(), VEC_809C2A64, VEC_809C2A7C);
        m_move->set_140(true);

        EGG::Vector3f vStack_64 = FUN_807230D4(local_4c);
        m_move->set_1c(vStack_64);
        m_move->setForwardFromKartObjMainRot(true);

        if (iVar2) {
            m_move->set_148(true);
            m_210->FUN_8072370C(0.0f, 1.0f, 40.0f, 3);
            m_state = State::Descending;
        }
    } break;
    case State::Descending: {
        bool iVar2 = m_210->FUN_80723AF8(1.0f);
        EGG::Vector3f local_4c = Interpolate(m_210->_0c(), VEC_809C2A7C, VEC_809C2A94);
        m_move->set_140(true);

        EGG::Vector3f vStack_7c = FUN_807230D4(local_4c);
        m_move->set_1c(vStack_7c);
        m_move->setForwardFromKartObjMainRot(true);

        if (iVar2) {
            m_state = State::Stay;
        }
    } break;
    case State::Stay: {
        m_move->set_140(false);
        EGG::Vector3f vStack_88 = FUN_807230D4(VEC_809C2A94);
        m_move->set_1c(vStack_88);
        m_move->setForwardFromKartObjPosDelta(false);

        if (!m_switchReverse->isOn() || local_40.length() > 2000.0f) {
            m_210->FUN_8072370C(0.0f, 1.0f, 70.0f, 3);
            m_move->set_148(false);
            m_ascendTimer = 0;
            m_state = State::Ascending;
        }
    } break;
    case State::Ascending: {
        bool iVar2 = m_210->FUN_80723AF8(1.0f);

        m_move->setRiseVel(++m_ascendTimer < 10 ? SLOW_RISE_VEL : FAST_RISE_VEL);
        m_move->set_150(true);

        if (iVar2) {
            m_nextStateId = 0;
        }
    } break;
    }

    m_move->calc();
}

/// @addr{0x807230D4}
EGG::Vector3f JugemUnit::FUN_807230D4(const EGG::Vector3f &v) {
    const EGG::Vector3f &vel1Dir = m_kartObj->move()->vel1Dir();
    const EGG::Quatf &mainRot = m_kartObj->mainRot();

    EGG::Vector3f vStack88 = mainRot.rotateVector(EGG::Vector3f::ez);
    const EGG::Vector3f &pos = m_kartObj->pos();
    EGG::Vector3f up = EGG::Vector3f::ey;
    EGG::Vector3f local_a0 = vel1Dir + vStack88;
    local_a0.normalise2();
    EGG::Vector3f right = up.cross(local_a0);
    right.normalise2();

    if (right.squaredLength() <= std::numeric_limits<f32>::epsilon()) {
        right = EGG::Vector3f::ex;
    }

    EGG::Vector3f forward = right.cross(up);
    forward.normalise2();

    EGG::Matrix34f mat;
    mat.setBase(0, right);
    mat.setBase(1, up);
    mat.setBase(2, forward);
    mat.setBase(3, pos);

    return mat.ps_multVector(v);
}

/// @addr{0x807232E4}
void JugemUnit::calcCollision() {
    constexpr f32 RADIUS = 150.0f;

    CollisionInfoPartial colInfo;
    KCLTypeMask maskOut;

    // We ignore the result of the collision check. This check is simply here so that any
    // resulting collisions can cause updates to nearby objects.
    std::ignore = CollisionDirector::Instance()->checkSpherePartialPush(RADIUS, m_pos,
            EGG::Vector3f::inf, KCL_TYPE_FLOOR, &colInfo, &maskOut, 0);
}

} // namespace Field
