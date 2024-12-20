#include "KartObjectManager.hh"

#include "game/kart/KartCollide.hh"
#include "game/kart/KartParamFileManager.hh"
#include "game/system/RaceConfig.hh"
#include "game/kart/KartObject.hh"

#include <ranges>

namespace Kart {

/// @addr{0x8058FEE0}
void KartObjectManager::init() {
    for (auto object : m_objects) {
        object->initImpl();
        object->prepare();
    }
}

/// @addr{0x8058FFE8}
void KartObjectManager::calc() {
    for (auto object : m_objects) {
        object->collide()->setTangentOff(EGG::Vector3f::zero);
        object->collide()->setMovement(EGG::Vector3f::zero);
        object->calcSub();
        object->calc();
    }
}

/// @addr{0x80590100}
KartObject *KartObjectManager::object(size_t i) {
    ASSERT(i < m_objects.size());
    return m_objects[i];
}

/// @addr{0x8058FAA8}
KartObjectManager *KartObjectManager::CreateInstance() {
    ASSERT(!s_instance);
    s_instance = new KartObjectManager;
    return s_instance;
}

/// @addr{0x8058FAF8}
void KartObjectManager::DestroyInstance() {
    ASSERT(s_instance);
    auto *instance = s_instance;
    s_instance = nullptr;
    delete instance;
}

KartObjectManager *KartObjectManager::Instance() {
    return s_instance;
}

/// @addr{0x8058FB2C}
KartObjectManager::KartObjectManager() {
    using std::ranges::views::iota;
    using std::ranges::views::zip;

    const auto &raceScenario = System::RaceConfig::Instance()->raceScenario();
    m_objects.reserve(raceScenario.players.size());
    KartParamFileManager::CreateInstance();
    for (auto [player, idx] : zip(raceScenario.players, iota(0))) {
        // useless memory moving? idc tho
        m_objects.emplace_back(KartObject::Create(player.character, player.vehicle, idx));
        m_objects[idx]->createModel();
    }
}

/// @addr{0x8058FDD4}
KartObjectManager::~KartObjectManager() {
    if (s_instance) {
        s_instance = nullptr;
        WARN("KartObjectManager instance not explicitly handled!");
    }

    KartParamFileManager::DestroyInstance();


    // If the proxy list is not cleared when we're done with the KartObjectManager, the list's
    // destructor calls delete on all of the links remaining in the list. Since the heaps are
    // gone by that point, this results in a segmentation fault. So, we clear the links here.
    KartObjectProxy::proxyList().clear();
}

KartObjectManager *KartObjectManager::s_instance = nullptr; ///< @addr{0x809C18F8}

} // namespace Kart
