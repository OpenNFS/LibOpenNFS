#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS::NFS4 {
    struct NEIGHBORDATA // info on neighbouring block numbers
    {
        int16_t blk, unknown;
    };

    struct NumObjs {
        uint32_t num;
        uint32_t unknown;
    };

    class TrkBlockHeader : public IRawData {
      public:
        TrkBlockHeader() = default;
        explicit TrkBlockHeader(std::ifstream &frd);
        void _SerializeOut(std::ofstream &frd) override;

        uint32_t sz[11];
        uint32_t unknown1[11];
        uint32_t nVertices;                           // total stored
        uint32_t nHiResVert, nLoResVert, nMedResVert; // #poly[...]+#polyobj
        uint32_t nVerticesDup, nObjectVert;
        uint32_t unknown2[2];
        glm::vec3 ptCentre;
        glm::vec3 ptBounding[4];
        NEIGHBORDATA nbdData[300]; // neighboring blocks
        NumObjs nobj[4];
        uint32_t nPolygons;
        glm::vec3 ptMin, ptMax;
        uint32_t unknown3;
        uint32_t nPositions;
        NumObjs nXobj;
        NumObjs nPolyobj;
        NumObjs nSoundsrc;
        NumObjs nLightsrc;
        uint32_t neighbors[8];

      protected:
        bool _SerializeIn(std::ifstream &frd) override;
    };
} // namespace LibOpenNFS::NFS4
