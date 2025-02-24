#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS::NFS3 {
    class TexBlock : public IRawData {
      public:
        TexBlock() = default;
        explicit TexBlock(std::ifstream &frd);
        void _SerializeOut(std::ofstream &ofstream) override;
        std::vector<glm::vec2> GetUVs();

        uint16_t width, height;
        uint32_t unknown1; // Blending related, hometown covered bridges godrays
        float corners[8];  // 4x planar coordinates == tiling?
        uint32_t unknown2;
        bool isLane;       // 1 if not a real texture (lane), 0 usually
        uint16_t qfsIndex; // index in QFS file

      private:
        bool _SerializeIn(std::ifstream &ifstream) override;
    };
} // namespace LibOpenNFS::NFS3
