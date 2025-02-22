#pragma once

#include <cstdint>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>

namespace LibOpenNFS::NFS4 {
#pragma pack(push, 1)
    struct Polygon {
        uint16_t vertex[4];
        uint16_t texture;
        uint16_t texflags;
        uint8_t animFlags; // 00 normally, 20 at end of row, 10 two-sided (HS  // used for animated textures //AnimInfo (Length : Period
        // AS LSB 3:HSB 5))

        [[nodiscard]] bool backface_cull() const {
            return (texflags & 0x8000) == 0;
        }
        [[nodiscard]] bool mirror_y() const {
            return (texflags & 0x0020) != 0;
        }
        [[nodiscard]] bool mirror_x() const {
            return (texflags & 0x0010) != 0;
        }
        [[nodiscard]] bool invert() const {
            return (texflags & 0x0008) != 0;
        }
        [[nodiscard]] bool rotate() const {
            return (texflags & 0x0004) != 0;
        }
        [[nodiscard]] bool is_lane() const {
            return (texflags & 0x0800) != 0;
        }
        [[nodiscard]] uint8_t texture_id() const {
            return (texture & 0x07FF);
        }
    };
#pragma pack(pop)

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

        enum Type {
            NORMAL_1 = 0x02,
            ANIMATED = 0x03,
            NORMAL_2 = 0x04,
            SPECIAL = 0x06
        };
    };
} // namespace LibOpenNFS::NFS4
