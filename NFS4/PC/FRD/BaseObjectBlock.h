#pragma once

#include "../Common.h"
#include "Common/IRawData.h"

namespace LibOpenNFS::NFS4 {
    class BaseObjectBlock : public IRawData {
      public:
        virtual ~BaseObjectBlock() {};
        explicit BaseObjectBlock(XObjHeader const &_header, std::ifstream &frd);
        bool _SerializeIn(std::ifstream &frd) override;
        void _SerializeOut(std::ofstream &frd) override;

        XObjHeader const &header;
        std::vector<glm::vec3> vertices;
        std::vector<uint32_t> shadingVertices;
        std::vector<Polygon> polygons;
    };
} // namespace LibOpenNFS::NFS4
