#include "TrackGeometry.h"

namespace LibOpenNFS {
    TrackGeometry::TrackGeometry()
        : Geometry("TrackModel",
                   std::vector<glm::vec3>(),
                   std::vector<glm::vec2>(),
                   std::vector<glm::vec3>(),
                   std::vector<unsigned int>(),
                   false,
                   glm::vec3(0, 0, 0)) {
    }
    TrackGeometry::TrackGeometry(std::vector<glm::vec3> const &vertices,
                                 std::vector<glm::vec3> const &normals,
                                 std::vector<glm::vec2> const &uvs,
                                 std::vector<uint32_t> const &textureIndices,
                                 std::vector<uint32_t> const &vertexIndices,
                                 std::vector<glm::vec4> const &shadingData,
                                 std::vector<uint32_t> const &debugData,
                                 glm::vec3 const centerPosition)
        : Geometry("TrackMesh", vertices, uvs, normals, vertexIndices, true, centerPosition),
          m_textureIndices(textureIndices), m_debugData(debugData) {
        // Index Shading data
        for (uint32_t m_vertex_index : vertexIndices) {
            m_shadingData.push_back(shadingData[m_vertex_index]);
        }
    }

    TrackGeometry::TrackGeometry(std::vector<glm::vec3> const &vertices,
                                 std::vector<glm::vec3> const &normals,
                                 std::vector<glm::vec2> const &uvs,
                                 std::vector<uint32_t> const &textureIndices,
                                 std::vector<uint32_t> const &vertexIndices,
                                 std::vector<glm::vec4> const &shadingData,
                                 glm::vec3 const centerPosition)
        : Geometry("TrackMesh", vertices, uvs, normals, vertexIndices, true, centerPosition),
          m_textureIndices(textureIndices) {
        // Fill the unused buffer with data
        m_debugData.resize(m_textureIndices.size());

        // Index Shading data
        for (auto const &vertexIndex : vertexIndices) {
            m_shadingData.push_back(shadingData[vertexIndex]);
        }
    }

} // namespace LibOpenNFS
