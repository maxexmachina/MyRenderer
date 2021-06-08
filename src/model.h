#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>

#include "geometry.h"
#include "../dependencies/tgaimage.h"

class Model {
public:
    explicit Model(const char *filename);
    ~Model() = default;
    [[nodiscard]] int nverts() const { return mVerts.size(); }
    [[nodiscard]] int nfaces() const { return mFaces.size(); }
    [[nodiscard]] Vec3f getVert(int i) const { return mVerts[i]; }
    [[nodiscard]] TGAColor getDiffuseColor(const Vec2i& uv) { return mDiffuseMap.get(uv.x, uv.y); }
    [[nodiscard]] std::vector<int> getFace(int idx);
    [[nodiscard]] Vec2i getUv(int iface, int nvert);
private:
    void loadTexture(const std::string& filename, const char *suffix, TGAImage& img);

    std::vector<Vec3f> mVerts;
    std::vector<std::vector<Vec3i>> mFaces; // attention, this Vec3i means vertex/uv/normal
    std::vector<Vec3f> mNorms;
    std::vector<Vec2f> mUv;
    TGAImage mDiffuseMap;
};

#endif //__MODEL_H__
