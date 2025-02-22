#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"
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
        enum Type {
            POLYGON_OBJECT = 0x01,
            ROAD_OBJECT1 = 0x02,
            ROAD_OBJECT2 = 0x03,
            ROAD_OBJECT3 = 0x04,
            SPECIAL = 0x06,
        };
    };


    struct SoundSource {
        glm::ivec3 refpoint;
        uint32_t type;
    };

    struct LightSource {
        glm::ivec3 refpoint;
        uint32_t type;
    };

    class TrkBlock final : public IRawData {
      public:
        TrkBlock() = default;
        explicit TrkBlock(TrkBlockHeader const &_header, std::ifstream &frd);
        void _SerializeOut(std::ofstream &frd) override;

        TrkBlockHeader header;
        std::vector<glm::vec3> vertices;
        std::vector<uint32_t> shadingVertices;
        std::vector<POLYVROADDATA> polyVroadData; // p
        std::vector<RefExtraObject> xobj;
        std::vector<RefExtraObject2> xobj2;
        std::vector<SoundSource> soundsrc;
        std::vector<LightSource> lightsrc;
        std::array<std::vector<Polygon>, 11> polygonData;
        std::vector<XObjChunk> xobjs;
        std::vector<PositionData> posData;

      protected:
        bool _SerializeIn(std::ifstream &frd) override;
    };
} // namespace LibOpenNFS::NFS4
