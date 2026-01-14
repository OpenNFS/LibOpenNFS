#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string_view>

#include "Common/NFSVersion.h"

namespace LibOpenNFS::NFS2 {
    struct VERT_HIGHP {
        int32_t x, z, y;
    };

    struct ANIM_POS {
        VERT_HIGHP position;
        int16_t unknown[4];
    };

    struct PC {
        struct VERT {
            int16_t x, z, y;
        };

        struct POLYGONDATA {
            int16_t texture;
            int16_t otherSideTex;
            uint8_t vertex[4];
        };

        // TODO: Move this GEO data back to GeoFile.h once I find a clean way to template a shared struct name
#pragma pack(push, 1)
        struct HEADER {
            uint32_t padding;     // Possible value: 0x00, 0x01, 0x02
            uint32_t unknown[32]; // useless list with values, which increase by 0x4 (maybe global offset list, which is needed for
            // calculating the position of the blocks)
            uint64_t unknown2; //  always 0x00
        };

        struct BLOCK_HEADER {
            uint32_t nVerts;
            uint32_t nPolygons;
            int32_t position[3]; // Absolute XYZ of the block
            uint16_t unknown;    // ? similar to the value in the list above
            uint16_t unknown1;   // ? similar to the value in the list above
            uint16_t unknown2;   // ? similar to the value in the list above
            uint16_t unknown3;   // ? similar to the value in the list above
            uint64_t padding[3]; // Always 0, 1, 1
        };

        struct POLY_3D {
            uint32_t texMapType;
            uint8_t vertex[4];
            char texName[4];
        };
#pragma pack(pop)
        // Mike Thompson CarEd disasm parts table for NFS2 Cars
        static constexpr std::array<std::string_view, 32> PART_NAMES = {{
            "High Additional Body Part",
            "High Main Body Part",
            "High Ground Part",
            "High Front Part",
            "High Back Part",
            "High Left Side Part",
            "High Right Side Part",
            "High Additional Left Side Part",
            "High Additional Right Side Part",
            "High Spoiler Part",
            "High Additional Part",
            "High Backlights",
            "High Front Right Wheel",
            "High Front Right Wheel Part",
            "High Front Left Wheel",
            "High Front Left Wheel Part",
            "High Rear Right Wheel",
            "High Rear Right Wheel Part",
            "High Rear Left Wheel",
            "High Rear Left Wheel Part",
            "Medium Additional Body Part",
            "Medium Main Body Part",
            "Medium Ground Part",
            "Low Wheel Part",
            "Low Main Part",
            "Low Side Part",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
            "Reserved",
        }};
    };

    struct PS1 {
        struct VERT {
            int16_t x, z, y, w;
        };

        struct POLYGONDATA {
            uint8_t texture;
            uint8_t otherSideTex;
            uint8_t vertex[4];
        };

        // TODO: Move this GEO data back to GeoFile.h once I find a clean way to template a shared struct name
#pragma pack(push, 1)
        struct BlockOffset {
            uint16_t type; // Always 0x0077, PS1 address? No, because in PC version too.
            uint16_t offset;
        };

        struct HEADER {
            uint32_t version; // Possible value: 0x00, 0x01, 0x02?
            BlockOffset blockOffsets[32];
            uint64_t padding;
        };

        struct BLOCK_HEADER {
            uint32_t nVerts;
            uint32_t unknown1; // Block type? Changes how many padding bytes there are uint16_t[(unknown1 + extraPadByte)*2]
            uint32_t nNormals; // Extra verts for higher LOD?
            uint32_t nPolygons;
            int32_t position[3];    // Absolute X,Y,Z reference
            int16_t unknown2[4][2]; // No clue
            uint64_t padding[3];    // Always 0, 1, 1
        };

        /*| Bit | Mask | Meaning |
          |-----|------|---------|
          | 0 | 0x01 | Triangle (vertex[2] == vertex[3]) |
          | 1 | 0x02 | Quad |
          | 2 | 0x04 | Mirrored/duplicate face |
          | 3 | 0x08 | Unknown (UV related?) |
          | 4 | 0x10 | Unknown |
          | 5 | 0x20 | Textured (vs flat-shaded) |*/
        struct POLY_3D {
            uint32_t type;
            uint16_t vertex_idx[4];
            uint16_t normal_idx[4];
            uint16_t uv_idx[4];
            char texName[4];
        };

        struct XBLOCK_1 {
            int16_t unknown[4];
        };

        struct XBLOCK_2 {
            int16_t unknown[4];
        };

        struct XBLOCK_3 {
            int16_t unknown[8];
        };

        struct XBLOCK_4 {
            int16_t unknown[5];
        };

        struct XBLOCK_5 {
            int16_t unknown[9];
        };
#pragma pack(pop)

        static constexpr std::array<std::string_view, 33> PART_NAMES = {{"High Additional Body Part",
                                                                         "High Main Body Part",
                                                                         "High Ground Part",
                                                                         "High Front Part",
                                                                         "High Rear Part",
                                                                         "High Left Side Part",
                                                                         "High Right Side Part",
                                                                         "High Additional Left Side Part",
                                                                         "High Additional Right Side Part",
                                                                         "High Front Rear Grilles",
                                                                         "High Extra Side Parts",
                                                                         "High Spoiler Part",
                                                                         "High Additional Part",
                                                                         "High Backlights",
                                                                         "High Front Right Wheel",
                                                                         "High Front Right Wheel Part",
                                                                         "High Front Left Wheel",
                                                                         "High Front Left Wheel Part",
                                                                         "High Rear Right Wheel",
                                                                         "High Rear Right Wheel Part",
                                                                         "High Rear Left Wheel",
                                                                         "High Rear Left Wheel Part",
                                                                         "Medium Additional Body Part",
                                                                         "Medium Main Body Part",
                                                                         "Medium Ground Part",
                                                                         "Wheel Positions",
                                                                         "Medium/Low Side Parts",
                                                                         "Low Main Part",
                                                                         "Low Side Part",
                                                                         "Headlight Positions",
                                                                         "Backlight Positions",
                                                                         "Reserved",
                                                                         "Reserved"}};
    };
} // namespace LibOpenNFS::NFS2