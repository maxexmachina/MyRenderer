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

void triangle(Vec2i& v0, Vec2i& v1, Vec2i& v2, TGAImage& img, const TGAColor& color) {
    std::array<Vec2i, 3> v{ v0, v1, v2 };
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

void triangle(std::array<Vec2i, 3>& v, TGAImage& img, const TGAColor& color) {
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

int main(int argc, char** argv) {
    TGAImage image{ width, height, TGAImage::RGB };

    std::array<Vec2i, 3> v0{ Vec2i{ 10, 70 }, Vec2i{ 50, 160 }, Vec2i{ 70, 80 } };
    std::array<Vec2i, 3> v1{ Vec2i{ 180, 50 }, Vec2i{ 150, 1 }, Vec2i{ 70, 180 } };
    std::array<Vec2i, 3> v2{ Vec2i{ 180, 150 }, Vec2i{ 120, 160 }, Vec2i{ 130, 180 } };

    triangle(v0, image, red);
    triangle(v1, image, white);
    triangle(v2, image, green);
//        triangle(v0[0], v0[1], v0[2], image, red);
//        triangle(v1[0], v1[1], v1[2], image, white);
//        triangle(v2[0], v2[1], v2[2], image, green);

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;

    return 0;
}