#include "geometry.h"

template <> template <> Vec3<int>::Vec3(const Vec3<float>& v) : x(static_cast<int>(v.x+.5)), y(static_cast<int>(v.y+.5)), z(static_cast<int>(v.z+.5)) {
}

template <> template <> Vec3<float>::Vec3(const Vec3<int>& v) : x(v.x), y(v.y), z(v.z) {
}
