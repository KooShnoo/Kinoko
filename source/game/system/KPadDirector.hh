#pragma once

#include "game/system/KPadController.hh"
#include <vector>

namespace System {

/// @brief The highest level abstraction for controller processing.
/// @addr{0x809BD70C}
class KPadDirector : EGG::Disposer {
public:
    void calc();
    void calcPads();
    void clear();
    void reset();
    void startGhostProxies();
    void endGhostProxies();

    [[nodiscard]] const KPadPlayer &playerInputs(size_t i) const;

    void pushGhostPad(const u8 *inputs, bool driftIsAuto);
    void pushHostPad(bool driftIsAuto);

    static KPadDirector *CreateInstance();
    static void DestroyInstance();
    [[nodiscard]] static KPadDirector *Instance();

private:
    KPadDirector();
    ~KPadDirector();

    std::vector<KPadPlayer> m_playerInputs;

    static KPadDirector *s_instance; ///< @addr{0x809BD70C}
};

} // namespace System
