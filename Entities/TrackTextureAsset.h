#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "TrackEntity.h"

namespace LibOpenNFS {
    class TrackTextureAsset {
    public:
        TrackTextureAsset() = default;
        explicit TrackTextureAsset(uint32_t id, uint32_t width, uint32_t height, std::string const& fileReference, std::string const& alphaFileReference);
        [[nodiscard]] std::vector<glm::vec2> ScaleUVs(std::vector<glm::vec2> const &uvs,
                                        bool invertU,
                                        bool invertV,
                                        uint8_t nRotate = 0,
                                        bool mirrorX = false,
                                        bool mirrorY = false) const;

        std::string fileReference;
        std::string alphaFileReference;
        uint32_t id{0};
        uint32_t width{0};
        uint32_t height{0};
        uint32_t layer{0};
        std::vector<glm::vec2> uvs;
        float minU{0.f};
        float minV{0.f};
        float maxU{0.f};
        float maxV{0.f};
    };
} // namespace LibOpenNFS
