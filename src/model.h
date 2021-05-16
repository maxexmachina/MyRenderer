#ifndef MYRENDERER_MODEL_H
#define MYRENDERER_MODEL_H

#include <vector>

#include "geometry.h"
#include "../dependencies/tgaimage.h"

class Model {
public:
    explicit Model(const char* filename);
    ~Model() = default;
    [[nodiscard]] int nverts() const { return mVerts.size(); }
    [[nodiscard]] int nfaces() const { return mFaces.size(); }
    [[nodiscard]] Vec3f getVert(int idx) const { return mVerts[idx]; }
    [[nodiscard]] TGAColor getDiffuseColor(Vec2i uv) { return mDiffuseMap.get(uv.x, uv.y); }
    [[nodiscard]] std::vector<int> getFace(int idx);
    [[nodiscard]] Vec2i getUv(int faceIdx, int nvert);
private:
    void loadTexture(const std::string& filename, const char* suffix, TGAImage& image);

    std::vector<Vec3f> mVerts;
    std::vector<std::vector<Vec3i>> mFaces;
    std::vector<Vec3f> mNorms;
    std::vector<Vec2f> mUv;
    TGAImage mDiffuseMap;
};

#endif //MYRENDERER_MODEL_H
