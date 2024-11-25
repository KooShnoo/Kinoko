#pragma once

#include <egg/math/Vector.hh>

namespace EGG {

/// @brief i have no clue lmao
class ProjThing {
public:
    ProjThing(Vector3f &a, Vector3f &b);
    bool isNegative(const Vector3f &pos);

private:
    Vector3f vec;
    f32 minusDot;
};

} // namespace EGG
