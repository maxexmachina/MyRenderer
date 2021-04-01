#include <iostream>
#include <array>
#include <algorithm>
#include <cmath>

#include "model.h"

static const TGAColor white{ 255, 255, 255, 255 };
static const TGAColor red{ 255, 0,   0,   255 };
static const TGAColor green{ 0, 255,   0,   255 };
static const TGAColor blue{0, 0, 255, 255};
static Model* model = nullptr;
static int* zbuffer = nullptr;
static const Vec3f lightDir{0, 0, -1};
static const auto width = 800;
static const auto height = 800;
static const auto depth = 255;

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
                     static_cast<float>(pts[0].y - p.y) };

    if (std::abs(u.z) < 1) {
        return Vec3f{ -1, 1, 1 };
    }
    return Vec3f{ 1.f - (u.x + u.y)/u.z, u.x/u.z, u.y/u.z };
}

void triangleOld(std::array<Vec3i, 3>& v, std::array<Vec2i, 3>& uv, TGAImage& image, float intensity, int* buffer) {
    if (v[0].y == v[1].y && v[0].y == v[2].y) return;

    if (v[0].y > v[1].y) { std::swap(v[0], v[1]); std::swap(uv[0], uv[1]); }
    if (v[0].y > v[2].y) { std::swap(v[0], v[2]); std::swap(uv[0], uv[2]); }
    if (v[1].y > v[2].y) { std::swap(v[1], v[2]); std::swap(uv[1], uv[2]); }

    const auto totalHeight = v[2].y - v[0].y;
    for (int i = 0; i < totalHeight; i++) {
        auto secondHalf = i > v[1].y - v[0].y || v[1].y == v[0].y;
        const auto segHeight = secondHalf ? v[2].y - v[1].y : v[1].y - v[0].y;

        const auto alpha = static_cast<float>(i) / totalHeight;
        const auto beta = static_cast<float>(i - (secondHalf ? v[1].y - v[0].y : 0)) / segHeight;

        auto a = v[0] + (v[2] - v[0]) * alpha;
        auto b = secondHalf ? v[1] + (v[2] - v[1]) * beta : v[0] + (v[1] - v[0]) * beta;

        auto uvA = uv[0] + (uv[2] - uv[0]) * alpha;
        auto uvB = secondHalf ? uv[1] + (uv[2] - uv[1]) * beta :
                uv[0] + (uv[1] - uv[0]) * beta;

        if (a.x > b.x) {
            std::swap(a, b);
            std::swap(uvA, uvB);
        }
        for (int j = a.x; j < b.x; j++) {
            const auto phi = b.x == a.x ? 1.0f : static_cast<float>(j - a.x)/static_cast<float>(b.x - a.x);

            const auto p = a + (b - a) * phi;
            const auto uvP = uvA + (uvB - uvA) * phi;

            const auto idx = static_cast<int>(p.x + p.y * width);
            if (buffer[idx] < p.z) {
                buffer[idx] = p.z;
                auto color = model->getDiffuseColor(uvP);
                image.set(p.x, p.y, TGAColor{ static_cast<uint8_t>(color.r * intensity),
                                              static_cast<uint8_t>(color.g * intensity),
                                              static_cast<uint8_t>(color.b * intensity), 255 });
            }
        }
    }
}

void triangle(std::array<Vec3f, 3>& pts, float *buffer, TGAImage& image, const TGAColor& color) {
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
            if (buffer[idx] < p.z) {
                buffer[idx] = p.z;
                image.set(p.x, p.y, color);
            }
        }
    }
}

Vec3i world2screen(Vec3f v) {
    return Vec3i((v.x + 1.0f) * width/2.0f + 0.5f,
                  (v.y + 1.0f) * height/2.0f + 0.5f,
                  (v.z + 1.0f) * depth/2.0f + 0.5f);
}

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model{ argv[1] };
    } else {
        model = new Model{ "../resources/african_head.obj" };
    }

    zbuffer = new int[ width * height ];
    std::fill(zbuffer, zbuffer + width * height, -std::numeric_limits<int>::max());

    {
        TGAImage image{width, height, TGAImage::RGB};
        for (int i = 0; i < model->nfaces(); i++) {
            const auto face = model->getFace(i);

            std::array<Vec3f, 3> worldCoords;
            std::array<Vec3i, 3> screenCoords;

            for (int j = 0; j < 3; j++) {
                const auto v = model->getVert(face[j]);
                worldCoords[j] = v;
                screenCoords[j] = world2screen(v);
            }

            const auto normal = ((worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0])).normalize();
            const auto intensity = lightDir * normal;
            if (intensity > 0) {
                std::array<Vec2i, 3> uv;
                for (int j = 0; j < 3; ++j) {
                    uv[j] = model->getUv(i, j);
                }

                triangleOld(screenCoords, uv, image, intensity, zbuffer);

            }
        }
        image.flip_vertically();
        image.write_tga_file("output.tga");
    }

    {
        { // dump z-buffer (debugging purposes only)
            TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
            for (int i=0; i<width; i++) {
                for (int j=0; j<height; j++) {
                    zbimage.set(i, j, TGAColor(zbuffer[i+j*width], 1));
                }
            }
            zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
            zbimage.write_tga_file("zbuffer.tga");
        }
    }


    delete model;
    delete[] zbuffer;
    return 0;
}