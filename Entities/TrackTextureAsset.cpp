#include "TrackTextureAsset.h"

#include <utility>

namespace LibOpenNFS {
    TrackTextureAsset::TrackTextureAsset(uint32_t const id, uint32_t const width, uint32_t const height, std::string const &fileReference,
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

    TrackTextureAsset::TrackTextureAsset(uint32_t const id, uint32_t const width, uint32_t const height, std::vector<uint8_t> pixelData)
        : data(std::move(pixelData)), id(id), width(width), height(height) {
    }

    std::vector<glm::vec2> TrackTextureAsset::ScaleUVs(std::vector<glm::vec2> const &uvs, bool const invertU, bool const invertV,
                                                       uint8_t const nRotate, bool const mirrorX, bool const mirrorY) const {
        std::vector<glm::vec2> temp_uvs = uvs;
        constexpr auto originTransform{glm::vec2(0.5f, 0.5f)};
        float const angle{(float)nRotate * 90.f};
        auto const uvRotationTransform{
            glm::mat2(cos(glm::radians(angle)), sin(glm::radians(angle)), -sin(glm::radians(angle)), cos(glm::radians(angle)))};

        for (auto &uv : temp_uvs) {
            if (nRotate != 0) {
                uv = ((uv - originTransform) * uvRotationTransform) + originTransform;
            }
            uv.x = (invertU ? (1 - uv.x) : uv.x) * maxU;
            uv.y = (invertV ? (1 - uv.y) : uv.y) * maxV;
        }
        if (mirrorY) {
            std::swap(temp_uvs[1].y, temp_uvs[2].y);
            temp_uvs[4].y = temp_uvs[2].y;
            std::swap(temp_uvs[0].y, temp_uvs[5].y);
        }
        if (mirrorX) {
            std::swap(temp_uvs[0].x, temp_uvs[1].x);
            temp_uvs[3].x = temp_uvs[0].x;
            std::swap(temp_uvs[2].x, temp_uvs[5].x);
            temp_uvs[4].x = temp_uvs[2].x;
        }

        return temp_uvs;
    }
} // namespace LibOpenNFS
