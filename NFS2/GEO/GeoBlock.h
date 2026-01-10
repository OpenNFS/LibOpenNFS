#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"

namespace LibOpenNFS {
    namespace NFS2 {
        template <typename Platform>
        class GeoBlock : IRawData {
        public:
            GeoBlock(std::ifstream &ifstream, uint32_t _partIdx);
            GeoBlock() = default;

            // Derived
            uint32_t partIdx{};

            // Raw File Data
            Platform::BLOCK_HEADER header;
            std::vector<glm::i16vec3> vertices;
            std::vector<typename Platform::POLY_3D> polygons;
        private:
            bool _SerializeIn(std::ifstream &ifstream) override;
            void _SerializeOut(std::ofstream &ofstream) override;
        };
    } // namespace NFS2
} // namespace LibOpenNFS
