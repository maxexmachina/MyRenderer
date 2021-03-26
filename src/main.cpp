#include <iostream>
#include <array>
#include <algorithm>

#include "model.h"
#include "../dependencies/tgaimage.h"

static const TGAColor white{ 255, 255, 255, 255 };
static const TGAColor red{ 255, 0,   0,   255 };
static const TGAColor green{ 0, 255,   0,   255 };
static Model* model;
static const int width = 200;
static const int height = 200;

void line(Vec2i p0, Vec2i p1, TGAImage& image, const TGAColor& color) {
    auto steep = false;
    if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x > p1.x) {
        std::swap(p0,p1);
    }

    const int dx = p1.x - p0.x;
    const int dy = p1.y - p0.y;
    const int derror2 = std::abs(dy)*2;
    int error2 = 0;
    int y = p0.y;
    for (int x = p0.x; x < p1.x; ++x) {
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

Vec3f barycentric(const std::array<Vec2i, 3>& pts, Vec2i p) {
    const Vec3f u = Vec3f{ static_cast<float>(pts[2].x - pts[0].x),
                     static_cast<float>(pts[1].x - pts[0].x),
                     static_cast<float>(pts[0].x - p.x) } ^
              Vec3f{ static_cast<float>(pts[2].y - pts[0].y),
                     static_cast<float>(pts[1].y - pts[0].y),
                     static_cast<float>(pts[0].y - p.y) };
    if (std::abs(u.z) < 1) return Vec3f{ -1, 1, 1 };
    return Vec3f{ 1.f - (u.x + u.y)/u.z, u.x/u.z, u.y/u.z };
}

void triangle(std::array<Vec2i, 3>& pts, TGAImage& image, const TGAColor& color) {
    Vec2i bboxmin{ image.get_width() - 1, image.get_height() - 1 };
    Vec2i bboxmax{ 0, 0 };
    Vec2i clamp{ image.get_width() - 1, image.get_height() - 1 };
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0,        std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec2i p;
    for (p.x = bboxmin.x; p.x < bboxmax.x; p.x++) {
        for (p.y = bboxmin.y; p.y < bboxmax.y; p.y++) {
            Vec3f bc = barycentric(pts, p);
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
            image.set(p.x, p.y, color);
        }
    }
}

int main(int argc, char** argv) {
    TGAImage image{ width, height, TGAImage::RGB };

    std::array<Vec2i, 3> pts{Vec2i{10, 10}, Vec2i{100, 30}, Vec2i{190, 160}};
    triangle(pts, image, red);

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;

    return 0;
}