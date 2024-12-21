#pragma once

#include "game/kart/KartObject.hh"

namespace Kart {

/// @brief Responsible for the lifecycle and calculation of KartObjects.
/// @addr{0x809C18F8}
/// @nosubgrouping
class KartObjectManager : EGG::Disposer {
public:
    void init();
    void calc();

    /// @beginGetters
    [[nodiscard]] KartObject *object(size_t i);
    /// @endGetters

    static KartObjectManager *CreateInstance();
    static void DestroyInstance();
    [[nodiscard]] static KartObjectManager *Instance();

private:
    KartObjectManager();
    ~KartObjectManager();

    std::vector<KartObject *> m_objects;

    static KartObjectManager *s_instance; ///< @addr{0x809C18F8}
};

} // namespace Kart
