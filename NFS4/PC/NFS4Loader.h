#pragma once

#include "Common/TextureUtils.h"
#include "Entities/Car.h"
#include "FCE/FceFile.h"
#include "FEDATA/FedataFile.h"

struct FLOATPT {
    float x, z, y;
};

struct INTPT {
    int32_t x, z, y;
};

struct NEIGHBORDATA // info on neighbouring block numbers
{
    int16_t blk, unknown;
};

struct POSITIONDATA // enumerate polygons which lie at center
{
    uint16_t polygon;
    unsigned char nPolygons;
    char unknown;
    int16_t extraNeighbor1, extraNeighbor2;
};

struct POLYVROADDATA // vroad data associated with a polygon
{
    unsigned char vroadEntry;
    unsigned char flags;
    unsigned char unknown[6];
    unsigned char hs_minmax[4];
    unsigned char hs_orphan[4];
    unsigned char hs_unknown;
};

struct VROADDATA // vroad vectors
{
    uint16_t xNorm, zNorm, yNorm;
    uint16_t xForw, zForw, yForw;
};

// WARNING: in the following 2 structures, don't rely on crossindex :
// it's not implemented in NFSHS's REFXOBJ, and due to a bug in T3ED
// refxobj.crossindex doesn't change properly if a polyobj's REFPOLYOBJ
// is deleted !

struct REFXOBJ // description of a block's XOBJects.
{
    INTPT pt;
    uint16_t unknown1;
    uint16_t globalno; // sequence number in all of the track's xobjs
    uint16_t unknown2;
    char crossindex; // position in first POLYOBJ chunk (0 if not in first chunk)
    char unknown3;
}; // !!! does not list the animated XOBJs

struct SOUNDSRC {
    INTPT refpoint;
    uint32_t type;
};

struct LIGHTSRC {
    INTPT refpoint;
    uint32_t type;
};

struct TRKBLOCK {
    FLOATPT ptCentre;
    FLOATPT ptBounding[4];
    uint32_t nVertices;                           // total stored
    uint32_t nHiResVert, nLoResVert, nMedResVert; // #poly[...]+#polyobj
    uint32_t nVerticesDup, nObjectVert;
    FLOATPT *vert; // the vertices
    uint32_t *unknVertices;
    NEIGHBORDATA nbdData[0x12C]; // neighboring blocks
    uint32_t nStartPos, nPositions;
    uint32_t nPolygons, nVRoad, nXobj, nPolyobj, nSoundsrc, nLightsrc;
    POSITIONDATA *posData;   // positions auint32_t track
    POLYVROADDATA *polyData; // polygon vroad references & flags
    VROADDATA *vroadData;    // vroad vectors
    REFXOBJ *xobj;
    SOUNDSRC *soundsrc;
    LIGHTSRC *lightsrc;
    FLOATPT hs_ptMin, hs_ptMax;
    uint32_t hs_neighbors[8];
};

struct POLYGONDATA {
    uint16_t vertex[4];
    uint16_t texture;
    uint16_t hs_texflags; // only used in road lane polygonblock ?
    unsigned char flags;  // 00 normally, 20 at end of row, 10 two-sided (HS  // used for animated textures //AnimInfo (Length : Period
    // AS LSB 3:HSB 5))
    unsigned char unknown2; // F9
};

typedef struct POLYGONDATA *LPPOLYGONDATA;

struct OBJPOLYBLOCK // a POLYOBJ chunk
{
    uint32_t n1;         // total number of polygons
    uint32_t n2;         // total number of objects including XOBJs
    uint32_t nobj;       // not stored in .FRD : number of type 1 objects
    uint32_t *types;     // when 1, there is an associated object; else XOBJ
    uint32_t *numpoly;   // size of each object (only for type 1 objects)
    LPPOLYGONDATA *poly; // the polygons themselves
};

struct POLYGONBLOCK {
    uint32_t sz[7], szdup[7];
    // 7 blocks == low res / 0 / med. res / 0 / high res / 0 / ??central
    LPPOLYGONDATA poly[7];
    OBJPOLYBLOCK obj[4]; // the POLYOBJ chunks
    // if not present, then all objects in the chunk are XOBJs
    // the 1st chunk is described anyway in the TRKBLOCK
};

struct ANIMDATA {
    INTPT pt;
    int16_t od1, od2, od3, od4;
};

struct XOBJDATA {
    uint32_t crosstype; // type 4, or more rarely 3 (animated)
    uint32_t crossno;   // obj number from REFXOBJ table in TRKBLOCK
    uint32_t unknown;
    // this section only for type 4 basic objects
    FLOATPT ptRef;
    uint32_t AnimMemory; // in HS, stores the unknown uint32_t for type 3 as well
    // this section only for type 3 animated objects
    uint16_t unknown3[9]; // 6 first are all alike; [6]==[8]=?; [7]=0
    // in HS, only 6 are used ; 6 = expected 4
    char type3, objno;               // type3==3; objno==index among all block's objects?
    uint16_t nAnimLength, AnimDelay; // JimDiabolo : The bigger the AnimDelay, that slower is the movement
    ANIMDATA *animData;
    // common section
    uint32_t nVertices;
    FLOATPT *vert; // the vertices
    uint32_t *unknVertices;
    uint32_t nPolygons;
    POLYGONDATA *polyData; // polygon data
};

struct XOBJBLOCK {
    uint32_t nobj;
    XOBJDATA *obj;
};

#pragma pack(1)
struct TEXTUREBLOCK // WARNING: packed but not byte-aligned !!!
{
    uint16_t width, height;
    uint32_t unknown1;
    float corners[8]; // 4x planar coordinates == tiling?
    uint32_t unknown2;
    char islane;      // 1 if not a real texture (lane), 0 usually
    uint16_t texture; // index in QFS file
};
#pragma pack()

// the associated COL file

#define XBID_TEXTUREINFO 2
#define XBID_STRUCT3D 8
#define XBID_OBJECT 7
#define XBID_OBJECT2 18
#define XBID_VROAD 15

struct XBHEAD {
    uint32_t size;
    uint16_t xbid;
    uint16_t nrec;
};

struct COLTEXTUREINFO {
    uint16_t texture;  // position in .QFS file
    uint16_t unknown1; // zero ?
    uint16_t unknown2; // texture offset ?
    uint16_t unknown3;
};

struct COLVERTEX {
    FLOATPT pt;       // relative coord
    uint32_t unknown; // like the unknVertices structures in FRD
};

struct COLPOLYGON {
    uint16_t texture;
    char v[4]; // vertices
};

struct COLSTRUCT3D {
    uint32_t size;
    uint16_t nVert, nPoly;
    COLVERTEX *vertex;
    COLPOLYGON *polygon;
};

struct COLOBJECT {
    uint16_t size;
    char type;     // 1 = basic object, 3 = animated ...
    char struct3D; // reference in previous block
    // type 1
    INTPT ptRef;
    // type 3
    uint16_t animLength;
    uint16_t unknown;
    ANIMDATA *animData; // same structure as in xobjs
};

struct COLVECTOR {
    signed char x, z, y, unknown;
};

struct COLVROAD {
    INTPT refPt;
    uint32_t unknown; // Unknown data
    COLVECTOR normal, forward, right;
    uint32_t leftWall, rightWall;
};

struct HS_VROADBLOCK { // HS's equivalent to a COLVROAD
    FLOATPT refPt;
    FLOATPT normal, forward, right;
    float leftWall, rightWall;
    float unknown1[2];
    uint32_t unknown2[5];
};

struct COLFILE {
    char collID[4];          // Header of file 'COLL'
    uint32_t version;        // Version number 11
    uint32_t fileLength;     // File length in bytes
    uint32_t nBlocks;        // Number of Xtra blocks in file
    uint32_t xbTable[5];     // Offsets of Xtra blocks
    XBHEAD textureHead;      // Record detailing texture table data
    COLTEXTUREINFO *texture; // Texture table
    XBHEAD struct3DHead;     // Record detailing struct3D table data
    COLSTRUCT3D *struct3D;   // Struct 3D table
    XBHEAD objectHead;       // Record detailing object table data
    COLOBJECT *object;       // Object table
    XBHEAD object2Head;      // Record detailing extra object data
    COLOBJECT *object2;      // Extra object data
    XBHEAD vroadHead;        // Unknown Record detailing unknown table data
    COLVROAD *vroad;         // Unknown table
    uint32_t *hs_extra;      // for the extra HS data in COLVROAD
};

namespace LibOpenNFS::NFS4 {
    glm::vec3 const NFS4_SCALE_FACTOR(-0.1, 0.1, 0.1f);
    class Loader {
      public:
        static Car LoadCar(std::string const &carBasePath, std::string const &carOutPath, NFSVersion version);
        // static TRACK LoadTrack(const std::string &track_base_path);

      private:
        static Car::MetaData _ParseAssetData(FceFile const &fceFile, FedataFile const &fedataFile, NFSVersion version);
        // static bool LoadFRD(const std::string &frd_path, const std::string &track_name, const std::shared_ptr<TRACK> &track);
        // static std::vector<TrackBlock> ParseTRKModels(const std::shared_ptr<TRACK> &track);
        // static Texture LoadTexture(TEXTUREBLOCK track_texture, const std::string &track_name);
    };

}; // namespace LibOpenNFS::NFS4
