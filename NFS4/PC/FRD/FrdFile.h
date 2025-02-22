#pragma once

#include "../../Common/IRawData.h"

#include "TrkBlock.h"
#include "TrkBlockHeader.h"
#include "VRoadBlock.h"

namespace LibOpenNFS::NFS4 {
    static constexpr uint8_t HEADER_LENGTH{28};

    typedef struct Polygon *LPPOLYGONDATA;

    struct OBJPOLYBLOCK // a POLYOBJ chunk
    {
        uint32_t n1;                   // total number of polygons
        uint32_t n2;                   // total number of objects including XOBJs
        uint32_t nobj;                 // not stored in .FRD : number of type 1 objects
        std::vector<uint32_t> types;   // when 1, there is an associated object; else XOBJ
        std::vector<uint32_t> numpoly; // size of each object (only for type 1 objects)
        LPPOLYGONDATA *poly;           // the polygons themselves
    };

    struct POLYGONBLOCK {
        uint32_t sz[7];
        // 7 blocks == low res / 0 / med. res / 0 / high res / 0 / ??central
        std::array<std::vector<Polygon>, 7> poly;
        OBJPOLYBLOCK obj[4]; // the POLYOBJ chunks
        // if not present, then all objects in the chunk are XOBJs
        // the 1st chunk is described anyway in the TRKBLOCK
    };

    struct ANIMDATA {
        glm::ivec3 pt;
        int16_t od1, od2, od3, od4;
    };

    struct XOBJDATA {
        uint32_t crosstype; // type 4, or more rarely 3 (animated)
        uint32_t crossno;   // obj number from REFXOBJ table in TRKBLOCK
        uint32_t unknown;
        // this section only for type 4 basic objects
        glm::vec3 ptRef;
        uint32_t AnimMemory; // in HS, stores the unknown uint32_t for type 3 as well
        // this section only for type 3 animated objects
        uint16_t unknown3[9]; // 6 first are all alike; [6]==[8]=?; [7]=0
        // in HS, only 6 are used ; 6 = expected 4
        char type3, objno;               // type3==3; objno==index among all block's objects?
        uint16_t nAnimLength, AnimDelay; // JimDiabolo : The bigger the AnimDelay, that slower is the movement
        std::vector<ANIMDATA> animData;
        // common section
        uint32_t nVertices;
        std::vector<glm::vec3> vert; // the vertices
        std::vector<uint32_t> shadingData;
        uint32_t nPolygons;
        std::vector<Polygon> polyData; // polygon data
    };

    struct XOBJBLOCK {
        uint32_t nobj;
        std::vector<XOBJDATA> obj;
    };

    struct TEXTUREBLOCK // WARNING: packed but not byte-aligned !!!
    {
        uint16_t width, height;
        uint32_t unknown1;
        float corners[8]; // 4x planar coordinates == tiling?
        uint32_t unknown2;
        char islane;      // 1 if not a real texture (lane), 0 usually
        uint16_t texture; // index in QFS file
    };

    class FrdFile : IRawData {
      public:
        FrdFile() = default;
        static bool Load(std::string const &frdPath, FrdFile &frdFile);
        static void Save(std::string const &frdPath, FrdFile &frdFile);

        // Raw File data
        uint32_t ptrspace[44];
        char header[HEADER_LENGTH];
        uint32_t nBlocks;
        uint32_t numVRoad;
        uint32_t nTextures;
        NFSVersion version;
        std::vector<VRoadBlock> vroadBlocks;
        std::vector<TrkBlockHeader> trackBlockHeaders;
        std::vector<TrkBlock> trackBlocks;
        uint32_t nGlobalObjects[2];
        std::vector<XObjChunk> globalObjects;

      private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS::NFS4
