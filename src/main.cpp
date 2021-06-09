#include <vector>
#include <limits>
#include <array>

#include "model.h"
#include "geometry.h"

static const TGAColor white{ 255, 255, 255, 255 };
static const TGAColor red{ 255, 0,   0,   255 };
static const TGAColor green{ 0, 255,   0,   255 };
static const TGAColor blue{ 0, 0, 255, 255 };
static Model* model = nullptr;
static int* zbuffer = nullptr;
static const Vec3f lightDir{ 0, 0, -1 };
static const auto width = 800;
static const auto height = 800;
static const auto depth = 255;
static const auto camera = Vec3f{ 0, 0, 3 };

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

void triangleOld(std::array<Vec3i, 3>& v, std::array<Vec2i, 3>& uv, TGAImage& image, float intensity, int *buffer) {

    if (v[0].y == v[1].y && v[0].y == v[2].y) return; // i dont care about degenerate triangles

    if (v[0].y > v[1].y) { std::swap(v[0], v[1]); std::swap(uv[0], uv[1]); }
    if (v[0].y > v[2].y) { std::swap(v[0], v[2]); std::swap(uv[0], uv[2]); }
    if (v[1].y > v[2].y) { std::swap(v[1], v[2]); std::swap(uv[1], uv[2]); }

    const auto totalHeight = v[2].y - v[0].y;
    for (int i=0; i < totalHeight; i++) {
        const auto secondHalf = i > v[1].y - v[0].y || v[1].y == v[0].y;
        const auto segmentHeight = secondHalf ? v[2].y - v[1].y : v[1].y - v[0].y;
        const auto alpha = static_cast<float>(i) / totalHeight;
        const auto beta  = static_cast<float>(i - (secondHalf ? v[1].y - v[0].y : 0)) / segmentHeight;
        auto A   =               v[0]  + Vec3f{ v[2] - v[0] } * alpha;
        auto B   = secondHalf ? v[1] + Vec3f{ v[2] - v[1] } * beta : v[0] + Vec3f{ v[1] - v[0]  } * beta;
        auto uvA =               uv[0] +      (uv[2] - uv[0]) * alpha;
        auto uvB = secondHalf ? uv[1] + (uv[2] - uv[1]) * beta : uv[0] + (uv[1] - uv[0]) * beta;
        if (A.x > B.x) { std::swap(A, B); std::swap(uvA, uvB); }
        for (int j = A.x; j <= B.x; j++) {
            const auto phi = B.x == A.x ? 1. : static_cast<float>(j - A.x)/static_cast<float>(B.x - A.x);
            const auto   P = Vec3f{ A }  + Vec3f{ B - A } * phi;
            const auto uvP =     uvA +   (uvB - uvA) * phi;
            const auto idx = static_cast<int>(P.x + P.y * width);
            if (buffer[idx] < P.z) {
                buffer[idx] = P.z;
                TGAColor color = model->getDiffuseColor(uvP);
                image.set(P.x, P.y, TGAColor{ static_cast<uint8_t>(color.r * intensity),
                                              static_cast<uint8_t>(color.g * intensity),
                                              static_cast<uint8_t>(color.b * intensity), 255 });
            }
        }
    }
}


Vec3i world2screen(Vec3f v) {
    return Vec3i{static_cast<int>((v.x + 1.0f) * width/2.0f + 0.5f),
                 static_cast<int>((v.y + 1.0f) * height/2.0f + 0.5f),
                 static_cast<int>((v.z + 1.0f) * depth/2.0f + 0.5f) };
}


Vec3f mat2vec(Matrix m) {
    return Vec3f{ m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0] };
}

Matrix vec2mat(const Vec3f& v) {
    Matrix res{ 4, 1 };
    res[0][0] = v.x;
    res[1][0] = v.y;
    res[2][0] = v.z;
    res[3][0] = 1.0f;
    return res;
}

Matrix viewport(int x, int y, int w, int h) {
    auto res = Matrix::eye(4);
    res[0][3] = x + w / 2.f;
    res[1][3] = y + h / 2.f;
    res[2][3] = depth / 2.f;

    res[0][0] = w / 2.f;
    res[1][1] = h / 2.f;
    res[2][2] = depth / 2.f;
    return res;
}

Matrix translation(const Vec3f& v) {
    auto res = Matrix::eye(4);
    res[0][3] = v.x;
    res[1][3] = v.y;
    res[2][3] = v.z;
    return res;
}

Matrix zoom(float factor) {
    auto res = Matrix::eye(4);
    res[0][0] = res[1][1] = res[2][2] = factor;
    return res;
}

Matrix rotX(float angle) {
    auto res = Matrix::eye(4);
    res[1][1] = res[2][2] = cosf(angle);
    res[1][2] = -sinf(angle);
    res[2][1] = sinf(angle);
    return res;
}

Matrix rotY(float angle) {
    auto res = Matrix::eye(4);
    res[0][0] = res[2][2] = cosf(angle);
    res[0][2] = -sinf(angle);
    res[2][0] = sinf(angle);
    return res;
}

Matrix rotZ(float angle) {
    auto res = Matrix::eye(4);
    res[0][0] = res[1][1] = cosf(angle);
    res[0][1] = -sinf(angle);
    res[1][0] = sinf(angle);
    return res;
}

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("../resources/african_head.obj");
    }

    zbuffer = new int[ width * height ];
    std::fill(zbuffer, zbuffer + width * height, std::numeric_limits<int>::min());

    { // draw the model
        auto projection = Matrix::eye(4);
        auto vp = viewport(width/8, height/8, width*3/4, height*3/4);
        projection[3][2] = -1.f / camera.z;



        TGAImage image(width, height, TGAImage::RGB);
        for (int i = 0; i < model->nfaces(); i++) {
            const auto face = model->getFace(i);

            std::array<Vec3i, 3> screen_coords;
            std::array<Vec3f, 3> world_coords;

            for (int j = 0; j < 3; j++) {
                Vec3f v = model->getVert(face[j]);
                screen_coords[j] = mat2vec(vp * projection * vec2mat(v));
                world_coords[j]  = v;
            }

            const auto normal = ((world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0])).normalize();
            float intensity = normal * lightDir;
            if (intensity > 0) {
                std::array<Vec2i, 3> uv;
                for (int j = 0; j < 3; j++) {
                    uv[j] = model->getUv(i, j);
                }
                triangleOld(screen_coords, uv, image, intensity, zbuffer);
            }
        }

        image.flip_vertically();
        image.write_tga_file("output.tga");
    }

    { // dump z-buffer
        TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                zbimage.set(i, j, TGAColor(zbuffer[i+j*width], 1));
            }
        }
        zbimage.flip_vertically();
        zbimage.write_tga_file("zbuffer.tga");
    }
    delete model;
    delete[] zbuffer;
    return 0;
}

