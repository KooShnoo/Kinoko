#include "ProjThing.hh"
namespace EGG {

ProjThing::ProjThing(Vector3f &a, Vector3f &b) {
    vec = b;
    // smth abt square root? but the return value is never refreenced??? this whole classs icks of smth im missing. i just dont get it??
    minusDot = -a.dot(b);
}

bool ProjThing::isNegative(const Vector3f &pos) {
    return minusDot + vec.dot(pos) <= 0;
}

} // namespace EGG
