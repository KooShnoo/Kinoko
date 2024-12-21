#pragma once

#include "game/field/obj/ObjectBase.hh"

namespace Field {

class ObjectNoImpl final : public ObjectBase {
public:
    ObjectNoImpl(const System::MapdataGeoObj &params);
    ~ObjectNoImpl();

    void load() override;
    void calcCollisionTransform() override {}
};

} // namespace Field
