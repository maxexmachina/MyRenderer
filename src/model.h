//
// Created by mavik on 21/03/2021.
//

#ifndef MYRENDERER_MODEL_H
#define MYRENDERER_MODEL_H

#include <vector>
#include "geometry.h"

class Model {
public:
    explicit Model(const char* filename);
    ~Model() = default;
    [[nodiscard]] int nverts() const { return static_cast<int>(mVerts.size()); }
    [[nodiscard]] int nfaces() const { return static_cast<int>(mFaces.size()); }
    [[nodiscard]] Vec3f vert(int idx) const { return mVerts[idx]; }
    [[nodiscard]] std::vector<int> face(int idx) const { return mFaces[idx]; }
    [[nodiscard]] auto faces() const { return mFaces; }

private:
    std::vector<std::vector<int>> mFaces;
    std::vector<Vec3f> mVerts;
};

#endif //MYRENDERER_MODEL_H
