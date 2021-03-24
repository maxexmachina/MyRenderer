//
// Created by mavik on 21/03/2021.
//

#ifndef MYRENDERER_GEOMETRY_H
#define MYRENDERER_GEOMETRY_H

#include <cmath>
#include <variant>
#include "../dependencies/fisqrt.h"

template <class T>
struct Vec2 {
    union {
        T raw[2];
        struct { T u, v; };
        struct { T x, y; };
    };
    Vec2() : u(0), v(0) {}
    Vec2(T _u, T _v) : u(_u), v(_v) {}

    inline Vec2<T> operator +(const Vec2<T>& vec) const { return Vec2<T>{ u + vec.u, v + vec.v }; }
    inline Vec2<T> operator -(const Vec2<T>& vec) const { return Vec2<T>{ u - vec.u, v - vec.v }; }
    inline Vec2<T> operator *(float f)            const { return Vec2<T>{ static_cast<T>(u * f), static_cast<T>(v * f) }; }

    friend std::ostream& operator<<(std::ostream& s, const Vec2<T>& v) {
        s << "(" << v.x << ", " << v.y << ")\n";
        return s;
    }
};

template <class T>
struct Vec3 {
    union {
        T raw[3];
        struct { T x, y, z; };
        struct { T ivert, iuv, inorm; };
    };
    Vec3() : x(0), y(0), z(0) {}
    Vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}

    inline Vec3<T> operator ^(const Vec3<T> &vec) const { return Vec3<T>{ y*vec.z-z*vec.y, z*vec.x-x*vec.z, x*vec.y-y*vec.x }; }
    inline Vec3<T> operator +(const Vec3<T> &vec) const { return Vec3<T>{ x+vec.x, y+vec.y, z+vec.z }; }
    inline Vec3<T> operator -(const Vec3<T> &vec) const { return Vec3<T>{ x-vec.x, y-vec.y, z-vec.z }; }
    inline Vec3<T> operator *(float f)            const { return Vec3<T>{ x*f, y*f, z*f }; }
    inline T       operator *(const Vec3<T> &vec) const { return x*vec.x + y*vec.y + z*vec.z; }

    [[nodiscard]] float norm() const { return std::sqrt(x*x + y*y + z*z); }
    [[nodiscard]] Vec3<T>& normalize(T l = 1) { *this = (*this) * l * Q_rsqrt(x*x + y*y + z*z); return *this; }

    friend std::ostream& operator<<(std::ostream& s, const Vec3<T>& v) {
        s << "(" << v.x << ", " << v.y << ")\n";
        return s;
    }

};

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;

#endif //MYRENDERER_GEOMETRY_H
