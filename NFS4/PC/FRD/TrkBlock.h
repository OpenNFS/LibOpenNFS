#pragma once

#include "../../Common/IRawData.h"
#include "TrkBlockHeader.h"
#include "XObjChunk.h"

namespace LibOpenNFS::NFS4 {
    // vroad data associated with a polygon
    struct POLYVROADDATA {
        uint8_t hs_minmax[4];
        uint8_t flags[5];
        uint8_t unknown1;
        uint16_t vroadEntry;
        glm::i16vec3 normal, forward;
    };

    struct PositionData // enumerate polygons which lie at center
    {
        uint16_t polygon;
        unsigned char nPolygons;
        char unknown;
        int16_t extraNeighbor1, extraNeighbor2;
    };

    struct RefExtraObject {
        glm::ivec3 pt;
        uint16_t unknown1;
        uint16_t globalno; // sequence number in all of the track's xobjs
        uint8_t unknown2[3];
        uint8_t collision;
    };

    struct RefExtraObject2 {
        uint16_t unknown;
        uint8_t type;
        uint8_t id;
        glm::ivec3 pt;
        uint8_t crossindex;
        uint8_t unknown2[3];
    };

    struct SoundSource {
        glm::ivec3 refpoint;
        uint32_t type;
    };

    struct LightSource {
        glm::ivec3 refpoint;
        uint32_t type;
    };

    struct POLYGONDATA {
        uint16_t vertex[4];
        uint16_t texture;
        uint16_t texflags; // only used in road lane polygonblock ?
        uint8_t animFlags; // 00 normally, 20 at end of row, 10 two-sided (HS  // used for animated textures //AnimInfo (Length : Period
        // AS LSB 3:HSB 5))
    };

    class TrkBlock final : public IRawData {
      public:
        TrkBlock() = default;
        explicit TrkBlock(TrkBlockHeader const &_header, std::ifstream &frd);
        void _SerializeOut(std::ofstream &frd) override;

        TrkBlockHeader header;
        std::vector<glm::vec3> vertices;
        std::vector<uint32_t> shadingVertices;
        std::vector<POLYVROADDATA> polyVroadData; // polygon vroad references & flags
        std::vector<RefExtraObject> xobj;
        std::vector<RefExtraObject2> xobj2;
        std::vector<SoundSource> soundsrc;
        std::vector<LightSource> lightsrc;
        std::vector<POLYGONDATA> polygonData;
        std::vector<XObjChunk> xobjs;
        std::vector<PositionData> posData; // positions auint32_t track

      protected:
        bool _SerializeIn(std::ifstream &frd) override;
    };
} // namespace LibOpenNFS::NFS4
