#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "TrackEntity.h"

namespace LibOpenNFS {
    class TrackTextureAsset {
      public:
        TrackTextureAsset() = default;
        // Legacy constructor with file references (for bitmap loading path)
        explicit TrackTextureAsset(uint32_t id, uint32_t width, uint32_t height, std::string const &fileReference,
                                   std::string const &alphaFileReference);
        // Direct pixel data constructor (for FSH direct loading path)
        explicit TrackTextureAsset(uint32_t id, uint32_t width, uint32_t height, std::vector<uint8_t> pixelData);

        [[nodiscard]] std::vector<glm::vec2> ScaleUVs(std::vector<glm::vec2> const &uvs, bool invertU, bool invertV, uint8_t nRotate = 0,
                                                      bool mirrorX = false, bool mirrorY = false) const;

        [[nodiscard]] bool HasPixelData() const {
            return !data.empty();
        }

        // Legacy file references (deprecated - use direct pixel data instead)
        std::string fileReference;
        std::string alphaFileReference;

        // Direct pixel data (RGBA format)
        std::vector<uint8_t> data;

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
