#include "KPadDirector.hh"
#include <game/system/KPadController.hh>

namespace System {

/// @addr{0x805238F0}
void KPadDirector::calc() {
    // calcPads();
    for (auto &playerInput : m_playerInputs) {
        playerInput.calc();
    }
}

// /// @addr{0x805237E8}
// void KPadDirector::calcPads() {
//     for (auto &playerInput : m_playerInputs) {
//     m_ghostController->calc();
//     m_hostController->calc();
// }

/// @addr{0x80523724}
void KPadDirector::clear() {
    m_playerInputs.clear();
}

/// @addr{0x80523690}
void KPadDirector::reset() {
    for (auto &playerInput : m_playerInputs) {
        playerInput.reset();
    }
}

/// @addr{0x80524580}
void KPadDirector::startGhostProxies() {
    for (auto &playerInput : m_playerInputs) {
        playerInput.startGhostProxy();
    }
}

/// @addr{0x805245DC}
void KPadDirector::endGhostProxies() {
    for (auto &playerInput : m_playerInputs) {
        playerInput.endGhostProxy();
    }
}

const KPadPlayer &KPadDirector::playerInputs(size_t i) const {
    return m_playerInputs[i];
}


/// @addr{0x8052453C}
void KPadDirector::pushGhostPad(const u8 *inputs, bool driftIsAuto) {
    m_playerInputs.emplace_back();
    m_playerInputs.back().setGhostController(new KPadGhostController, inputs, driftIsAuto);
}

void KPadDirector::pushHostPad(bool driftIsAuto) {
    m_playerInputs.emplace_back();
    m_playerInputs.back().setHostController(new KPadHostController, driftIsAuto);
}

/// @addr{0x8052313C}
KPadDirector *KPadDirector::CreateInstance() {
    ASSERT(!s_instance);
    return s_instance = new KPadDirector;
}

/// @addr{0x8052318C}
void KPadDirector::DestroyInstance() {
    ASSERT(s_instance);
    auto *instance = s_instance;
    s_instance = nullptr;
    delete instance;
}

KPadDirector *KPadDirector::Instance() {
    return s_instance;
}

/// @addr{0x805232F0}
KPadDirector::KPadDirector() = default;
// KPadDirector::KPadDirector() {
    // m_ghostController = new KPadGhostController;
    // m_hostController = new KPadHostController;
// }

/// @addr{0x805231DC}
KPadDirector::~KPadDirector() {
    if (s_instance) {
        s_instance = nullptr;
        WARN("KPadDirector instance not explicitly handled!");
    }
}

KPadDirector *KPadDirector::s_instance = nullptr; ///< @addr{0x809BD70C}

} // namespace System
