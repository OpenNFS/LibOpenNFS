#include "TrackTextureAsset.h"

namespace LibOpenNFS {
    TrackTextureAsset::TrackTextureAsset(uint32_t const id,
                                         uint32_t const width,
                                         uint32_t const height,
                                         std::string const &fileReference,
                                         std::string const &alphaFileReference) {
        this->id = id;
        this->width = width;
        this->height = height;
        this->fileReference = fileReference;
        this->alphaFileReference = alphaFileReference;
        this->layer = 0;
        this->minU = 0.f;
        this->minV = 0.f;
        this->maxU = 0.f;
        this->maxV = 0.f;
    }

    std::vector<glm::vec2> TrackTextureAsset::ScaleUVs(std::vector<glm::vec2> const &uvs,
                                                       bool const inverseU,
                                                       bool const inverseV,
                                                       uint8_t const nRotate,
                                                       bool const mirrorX,
                                                       bool const mirrorY) const {
        std::vector<glm::vec2> temp_uvs = uvs;
        constexpr auto originTransform{glm::vec2(0.5f, 0.5f)};
        float const angle{(float)nRotate * 90.f};
        auto const uvRotationTransform{
            glm::mat2(cos(glm::radians(angle)), sin(glm::radians(angle)), -sin(glm::radians(angle)), cos(glm::radians(angle)))};

        // NFS2
        // ROAD: uint8_t nRotate = (textureFlags >> 11) & 3; // 8,11 ok
        // XOBJ: nRotate = 0
        // inverseU = textureAlignment[8];
        // inverseV = textureAlignment[9];

        // NFS4
        if (mirrorY) {
            std::swap(temp_uvs[1].y, temp_uvs[2].y);
            std::swap(temp_uvs[2].y, temp_uvs[1].y);
            std::swap(temp_uvs[0].y, temp_uvs[3].y);
            std::swap(temp_uvs[3].y, temp_uvs[0].y);
        }
        if (mirrorX) {
            std::swap(temp_uvs[0].x, temp_uvs[1].x);
            std::swap(temp_uvs[1].x, temp_uvs[0].x);
            std::swap(temp_uvs[2].x, temp_uvs[3].x);
            std::swap(temp_uvs[3].x, temp_uvs[2].x);
        }

        for (auto &uv : temp_uvs) {
            uv.x = (inverseU ? (1 - uv.x) : uv.x) * maxU;
            uv.y = (inverseV ? (1 - uv.y) : uv.y) * maxV;

            if (nRotate != 0) {
                uv = ((uv - originTransform) * uvRotationTransform) + originTransform;
            }
        }

        return temp_uvs;
    }
} // namespace LibOpenNFS
