#include <iostream>
#include <array>
#include <algorithm>
#include <cmath>

#include "model.h"
#include "../dependencies/tgaimage.h"

static const TGAColor white{ 255, 255, 255, 255 };
static const TGAColor red{ 255, 0,   0,   255 };
static const TGAColor green{ 0, 255,   0,   255 };
static const TGAColor blue{0, 0, 255, 255};
static Model* model;
static const auto width = 800;
static const auto height = 800;

void line(Vec2i p0, Vec2i p1, TGAImage& image, const TGAColor& color) {
    auto x0 = p0.x;
    auto x1 = p1.x;
    auto y0 = p0.y;
    auto y1 = p1.y;

    auto steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    const auto dx = x1 - x0;
    const auto dy = y1 - y0;
    const auto derror2 = std::abs(dy) * 2;
    auto error2 = 0;
    auto y = y0;

    for (int x = x0; x < x1; ++x) {
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx *2;
        }
    }

}

Vec3f barycentric(const std::array<Vec3f, 3>& pts, Vec3f p) {
    const auto u = Vec3f{ static_cast<float>(pts[2].x - pts[0].x),
                     static_cast<float>(pts[1].x - pts[0].x),
                     static_cast<float>(pts[0].x - p.x) } ^
              Vec3f{ static_cast<float>(pts[2].y - pts[0].y),
                     static_cast<float>(pts[1].y - pts[0].y),
                     static_cast<float>(pts[0].y - p.y)
    };

    if (std::abs(u.z) < 1) {
        return Vec3f{ -1, 1, 1 };
    }
    return Vec3f{ 1.f - (u.x + u.y)/u.z, u.x/u.z, u.y/u.z };
}

void triangleOld(std::array<Vec2i, 3>& v, TGAImage& img, const TGAColor& color) {
    std::sort(v.begin(), v.end(), [](auto a, auto b) { return a.y < b.y; });
    const auto totalHeight = v[2].y - v[0].y;
    for (int i = 0; i < totalHeight; i++) {
        auto secondHalf = i > v[1].y - v[0].y || v[1].y == v[0].y;
        const auto segHeight = secondHalf ? v[2].y - v[1].y : v[1].y - v[0].y;
        const auto alpha = static_cast<float>(i) / totalHeight;
        const auto beta = static_cast<float>(i - (secondHalf ? v[1].y - v[0].y : 0)) / segHeight;
        auto a = v[0] + (v[2] - v[0]) * alpha;
        auto b = secondHalf ? v[1] + (v[2] - v[1]) * beta : v[0] + (v[1] - v[0]) * beta;
        if (a.x > b.x) std::swap(a, b);
        for (int j = a.x; j < b.x; j++) {
            img.set(j, v[0].y + i, color);
        }
    }
}

void triangle(std::array<Vec3f, 3>& pts, float *zbuffer, TGAImage& image, const TGAColor& color) {
    Vec2f bboxmin{ std::numeric_limits<float>::max(),  std::numeric_limits<float>::max() };
    Vec2f bboxmax{ -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max() };
    Vec2f clamp{ static_cast<float>(image.get_width()-1), static_cast<float>(image.get_height()-1) };

    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }

    Vec3f p;
    for (p.x=bboxmin.x; p.x <= bboxmax.x; p.x++) {
        for (p.y=bboxmin.y; p.y <= bboxmax.y; p.y++) {
            auto bcScreen  = barycentric(pts, p);

            if (bcScreen.x < 0 || bcScreen.y < 0 || bcScreen.z < 0) continue;

            p.z = 0;
            for (int i=0; i<3; i++) {
                p.z += pts[i][2] * bcScreen[i];
            }
            const auto idx = static_cast<int>(p.x + p.y * width);
            if (zbuffer[idx] < p.z) {
                zbuffer[idx] = p.z;
                image.set(p.x, p.y, color);
            }
        }
    }
}

Vec3f world2screen(Vec3f v) {
    return Vec3f( lroundf((v.x + 1.0f) * width/2.0f + 0.5f), lroundf((v.y+1.0f) * height/2.0f + 0.5f), v.z );
}

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model{ argv[1] };
    } else {
        model = new Model{ "../resources/obj/african_head.obj" };
    }

    auto* zbuffer = new float[ width * height ];
    std::fill(zbuffer, zbuffer + width * height, -std::numeric_limits<float>::max());

    TGAImage image{ width, height, TGAImage::RGB };
    const Vec3f lightDir{0, 0, -1 };
    for (const auto& face: model->faces()) {
        std::array<Vec3f, 3> worldCoords;
        std::array<Vec3f, 3> screenCoords;
        for (int i=0; i<3; i++) {
            const auto v = model->vert(face[i]);
            worldCoords[i] = v;
            screenCoords[i] = world2screen(v);
        }

        const auto normal = ((worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0])).normalize();
        const auto intensity = lightDir * normal;
        if (intensity > 0) {
            triangle(screenCoords, zbuffer, image, TGAColor{ static_cast<uint8_t>(255 * intensity), static_cast<uint8_t>(255 * intensity), static_cast<uint8_t>(255 * intensity), 255 });
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    delete[] zbuffer;
    return 0;
}