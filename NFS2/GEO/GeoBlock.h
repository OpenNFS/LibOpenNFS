#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"

namespace LibOpenNFS::NFS2 {
    template <typename Platform> class GeoBlock : IRawData {
      public:
        GeoBlock(std::ifstream &ifstream, uint32_t _partIdx);
        GeoBlock() = default;

        // Derived
        uint32_t partIdx{};

        // Raw File Data
        Platform::BLOCK_HEADER header;
        std::vector<glm::i16vec3> vertices;
        std::vector<typename Platform::POLY_3D> polygons;
        std::vector<glm::i16vec3> normals; // PS1 only

        PS1::XBLOCK_1 xblock_1;
        PS1::XBLOCK_2 xblock_2;
        PS1::XBLOCK_3 xblock_3;
        PS1::XBLOCK_4 xblock_4;
        PS1::XBLOCK_5 xblock_5;

      private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS::NFS2
