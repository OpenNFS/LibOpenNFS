#pragma once

#include "BaseLight.h"
#include <vector>

namespace LibOpenNFS {
    class TrackLight final : public BaseLight {
      public:
        TrackLight(uint32_t entityID, glm::vec3 position, uint32_t nfsType);
        TrackLight(uint32_t entityID,
                   glm::vec3 position,
                   glm::vec4 colour,
                   uint32_t unknown1,
                   uint32_t unknown2,
                   uint32_t unknown3,
                   float unknown4);
        ~TrackLight() override = default;
        // NFS3 and 4 light data stored in TR.ini [track glows]
        uint32_t nfsType;
        uint32_t unknown1, unknown2, unknown3;
        float unknown4;

        static constexpr float kLightSize = 3.0f;
        std::vector<uint32_t> indices = {0, 1,
                                         2,        // first triangle (bottom left - top left - top right)
                                         0, 2, 3}; // second triangle (bottom left - top right - bottom right)
        std::vector<glm::vec3> const verts = {
            glm::vec3(-kLightSize, -kLightSize, 0), // bottom left corner
            glm::vec3(-kLightSize, kLightSize, 0),  // top left corner
            glm::vec3(kLightSize, kLightSize, 0),   // top right corner
            glm::vec3(kLightSize, -kLightSize, 0),  // bottom right corner
        };
        std::vector<glm::vec3> const normals = {
            glm::vec3(0, 0, 0),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 0, 0),
        };
        std::vector<glm::vec2> uvs = {glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f),
                                      glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f)};
        //, Geometry("light", verts, uvs, normals, indices, true, position)
    };
} // namespace LibOpenNFS
