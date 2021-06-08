#ifndef MYRENDERER_GEOMETRY_H
#define MYRENDERER_GEOMETRY_H

#include <iostream>
#include <cassert>
#include <cmath>

#include "../dependencies/fisqrt.h"

template <class T>
struct Vec2 {
    T x, y;

    Vec2<T>() : x(T()), y(T()) {}
    Vec2<T>(T _x, T _y) : x(_x), y(_y) {}
    Vec2<T>(const Vec2<T>& v) : x(v.x), y(v.y) {}
    Vec2<T>& operator=(const Vec2<T>& v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    T& operator[](const int i) { assert(i >= 0 && i < 2); if (x==0) return x; else return y; }
    Vec2<T> operator+(const Vec2<T>& V) const { return Vec2<T>{ x+V.x, y+V.y }; }
    Vec2<T> operator-(const Vec2<T>& V) const { return Vec2<T>{ x-V.x, y-V.y }; }
    Vec2<T> operator*(float f)          const { return Vec2<T>{ static_cast<T>(x*f), static_cast<T>(y*f) }; }

    friend std::ostream& operator<<(std::ostream& s, Vec2<T>& v) {
        s << "(" << v.x << ", " << v.y << ")\n";
        return s;
    }
};

template <class T>
struct Vec3 {
    T x, y, z;

    Vec3<T>() : x(T()), y(T()), z(T()) { }
    Vec3<T>(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
    template <class U> Vec3<T>(const Vec3<U>& v);
    Vec3<T>(const Vec3<T>& v) : x(v.x), y(v.y), z(v.z) {}
    Vec3<T>& operator =(const Vec3<T>& v) {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    T& operator[](const int i) { assert(i >=0 && i < 3); if (i==0) return x; else if (i==1) return y; else return z; }

    Vec3<T> operator^(const Vec3<T>& v) const { return Vec3<T>{ y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x }; }
    Vec3<T> operator+(const Vec3<T>& v) const { return Vec3<T>{ x+v.x, y+v.y, z+v.z }; }
    Vec3<T> operator-(const Vec3<T>& v) const { return Vec3<T>{ x-v.x, y-v.y, z-v.z }; }
    Vec3<T> operator*(float f)          const { return Vec3<T>{ static_cast<T>(x*f), static_cast<T>(y*f), static_cast<T>(z*f) }; }
    T       operator*(const Vec3<T>& v) const { return x*v.x + y*v.y + z*v.z; }

    [[nodiscard]] float norm () const { return std::sqrt(x*x+y*y+z*z); }
    [[nodiscard]] Vec3<T> & normalize(T l=1) { *this = (*this) * l * Q_rsqrt(x*x + y*y + z*z); return *this; }

    friend std::ostream& operator<<(std::ostream& s, Vec3<T>& v) {
        s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
        return s;
    }
};

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;

template <> template <> Vec3<int>::Vec3(const Vec3<float> &v);
template <> template <> Vec3<float>::Vec3(const Vec3<int> &v);

#endif //MYRENDERER_GEOMETRY_H
