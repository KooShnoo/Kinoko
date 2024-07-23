#include "KPadDirector.hh"
#include <cassert>
#include <cstdio>
#include <game/system/KPadController.hh>

namespace System {

/// @addr{0x805238F0}
void KPadDirector::calc() {
    calcPads();
    m_playerInput.calc();
}

/// @addr{0x805237E8}
void KPadDirector::calcPads() {
    assert(m_controller);
    m_controller->calc();
}

/// @addr{0x80523724}
void KPadDirector::clear() {}

/// @addr{0x80523690}
void KPadDirector::reset() {
    m_playerInput.reset();
}

/// @addr{0x80524580}
void KPadDirector::startGhostProxies() {
    m_playerInput.startGhostProxy();
}

/// @addr{0x805245DC}
void KPadDirector::endGhostProxies() {
    m_playerInput.endGhostProxy();
}

const KPadPlayer &KPadDirector::playerInput() const {
    return m_playerInput;
}

/// @addr{none lol, i made it up}
void KPadDirector::setController(KPadController *controller) {
    // Bruv y r there two of theses, like fr fr y ?? did nintenedo do this? 
    // shaking my smh haed mman
    Instance()->m_controller = controller;
    Instance()->m_playerInput.setController(controller);
}

/// @addr{0x8052453C}
void KPadDirector::setGhostPad(const u8 *inputs, bool driftIsAuto) {
    const auto controller = new KPadGhostController;
    m_playerInput.setGhostController(controller, inputs, driftIsAuto);
    m_controller = controller;
}

/// @addr{0x8052313C}
KPadDirector *KPadDirector::CreateInstance() {
    assert(!s_instance);
    return s_instance = new KPadDirector;
}

/// @addr{0x8052318C}
void KPadDirector::DestroyInstance() {
    assert(s_instance);
    delete s_instance;
    s_instance = nullptr;
}

KPadDirector *KPadDirector::Instance() {
    return s_instance;
}

/// @addr{0x805232F0}
KPadDirector::KPadDirector() = default;
/// @addr{0x805231DC}
KPadDirector::~KPadDirector() = default;

KPadDirector *KPadDirector::s_instance = nullptr; ///< @addr{0x809BD70C}

} // namespace System
