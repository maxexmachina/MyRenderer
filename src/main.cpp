#include <iostream>
#include <array>
#include <algorithm>

#include "model.h"
#include "../dependencies/tgaimage.h"

static const TGAColor white{ 255, 255, 255, 255 };
static const TGAColor red{ 255, 0,   0,   255 };
static const TGAColor green{ 0, 255,   0,   255 };
static Model* model;
static const int width = 800;
static const int height = 800;

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

void triangle_old(std::array<Vec2i, 3>& v, TGAImage& img, const TGAColor& color) {
    std::sort(v.begin(), v.end(), [](auto a, auto b) { return a.y < b.y; });
    const auto total_height = v[2].y - v[0].y;
    for (int i = 0; i < total_height; i++) {
        auto second_half = i > v[1].y - v[0].y || v[1].y == v[0].y;
        const auto seg_height = second_half ? v[2].y - v[1].y : v[1].y - v[0].y;
        const auto alpha = static_cast<float>(i)/total_height;
        const auto beta = static_cast<float>(i - (second_half ? v[1].y - v[0].y : 0))/seg_height;
        auto a = v[0] + (v[2] - v[0]) * alpha;
        auto b = second_half ? v[1] + (v[2] - v[1]) * beta : v[0] + (v[1] - v[0]) * beta;
        if (a.x > b.x) std::swap(a, b);
        for (int j = a.x; j < b.x; j++) {
            img.set(j, v[0].y + i, color);
        }
    }
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

    if (argc == 2) {
        model = new Model{ argv[1] };
    } else {
        model = new Model{"../resources/obj/african_head.obj"};
    }

    const Vec3f light_dir{ 0, 0, -1 };
    for (const auto& face : model->faces()) {
        std::array<Vec2i, 3> screen_coords;
        std::array<Vec3f, 3> world_coords;
        for (int i = 0; i < 3; ++i) {
            const auto v = model->vert(face[i]);
            screen_coords[i] = Vec2i{ static_cast<int>((v.x + 1)*width/2), static_cast<int>((v.y + 1)*height/2) };
            world_coords[i] = v;
        }
        const auto normal = ((world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0])).normalize();
        const auto intensity = light_dir * normal;
        if (intensity > 0) {
            triangle(screen_coords, image, TGAColor{ static_cast<uint8_t>(255 * intensity), static_cast<uint8_t>(255 * intensity), static_cast<uint8_t>(255 * intensity), 255 });
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;

    return 0;
}