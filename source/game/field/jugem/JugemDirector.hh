#pragma once

#include "game/field/jugem/JugemUnit.hh"

namespace Field {

class JugemDirector {
    friend class Host::Context;

public:
    void init();
    void calc();

    static JugemDirector *CreateInstance();
    [[nodiscard]] static JugemDirector *Instance();
    static void DestroyInstance();

private:
    JugemDirector();
    ~JugemDirector();

    void createUnits();

    JugemUnit *m_unit; ///< Assumes 1 Lakitu because 1 player

    static JugemDirector *s_instance;
};

} // namespace Field
