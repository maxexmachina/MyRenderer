
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

#include "model.h"

Model::Model(const char *filename) : mVerts(), mFaces(), mNorms(), mUv() {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) {
        std::cerr << "Error loading model from " << filename << ": " << std::strerror(errno) << '\n';
    }
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss{ line };
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++) {
                iss >> v[i];
            }
            mVerts.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f n;
            for (int i = 0; i < 3; ++i) {
                iss >> n[i];
            }
            mNorms.push_back(n);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f uv;
            for (int i = 0; i < 2; ++i) {
                iss >> uv[i];
            }
            mUv.push_back(uv);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<Vec3i> f;
            Vec3i tmp;
            iss >> trash;
            while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
                // Indicies start at 1
                for (int i = 0; i < 3; ++i) {
                    tmp[i]--;
                }
                f.push_back(tmp);
            }
            mFaces.push_back(f);
        }
    }
    std::cerr << "# v# " << mVerts.size()
              << " f# "  << mFaces.size() << " vt# "
              << mUv.size() << " vn# " << mNorms.size() << '\n';

    loadTexture(filename, "_diffuse.tga", mDiffuseMap);
}

// Returns vertices of a face
std::vector<int> Model::getFace(int idx) {
    std::vector<int> face;
    for (auto& vec: mFaces[idx]) {
        face.push_back(vec[0]);
    }
    return face;
}

void Model::loadTexture(const std::string& filename, const char *suffix, TGAImage& image) {
    std::string texfile{ filename };
    const auto dot = texfile.find_last_of('.');
    if (dot != std::string::npos) {
        texfile = texfile.substr(0, dot) + std::string{ suffix };
        std::cerr << "Texture file " << texfile << " loading " << (image.read_tga_file(texfile.c_str()) ? "ok" : "failed") << '\n';
        image.flip_vertically();
    }
}

Vec2i Model::getUv(int faceIdx, int nvert) {
    const auto idx = mFaces[faceIdx][nvert][1];
    return Vec2i{ static_cast<int>(mUv[idx].x * mDiffuseMap.get_width()),
                  static_cast<int>(mUv[idx].y * mDiffuseMap.get_height()) };
}