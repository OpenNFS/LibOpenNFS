#pragma once

#include "Geometry.h"

namespace LibOpenNFS {
    class TrackGeometry : public Geometry {
    public:
        TrackGeometry();
        TrackGeometry(const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals, const std::vector<glm::vec2> &uvs, const std::vector<uint32_t> &textureIndices,
                      const std::vector<uint32_t> &vertexIndices, const std::vector<glm::vec4> &shadingData, const std::vector<uint32_t> &debugData, glm::vec3 centerPosition);
        TrackGeometry(const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals, const std::vector<glm::vec2> &uvs, const std::vector<uint32_t> &textureIndices,
                      const std::vector<uint32_t> &vertexIndices, const std::vector<glm::vec4> &shadingData, glm::vec3 centerPosition);
        ~TrackGeometry() override = default;
        std::vector<uint32_t> m_textureIndices;
        std::vector<glm::vec4> m_shadingData;
        std::vector<uint32_t> m_debugData;
    };
} // namespace LibOpenNFS
