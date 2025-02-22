#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS::NFS4 {
    struct XObjHeader {
        uint32_t type;
        uint32_t index;
        uint32_t unknown1;
        glm::vec3 pt;
        uint32_t size;
        uint32_t unknown2;
        uint32_t nVertices;
        uint32_t unknown3[2];
        uint32_t nPolygons;
        uint32_t unknown4;
    };

    class XObjChunk : public IRawData {
      public:
        XObjChunk() = default;
        explicit XObjChunk(uint32_t _nObjects, std::ifstream &frd);
        void _SerializeOut(std::ofstream &frd) override;

        uint32_t nObjects;

        std::vector<XObjHeader> objectHeaders;


      protected:
        bool _SerializeIn(std::ifstream &frd) override;
    };
} // namespace LibOpenNFS::NFS4
