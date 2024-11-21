#include "TrackGeometry.h"

namespace LibOpenNFS {
    TrackGeometry::TrackGeometry() :
        Geometry("TrackModel", std::vector<glm::vec3>(), std::vector<glm::vec2>(), std::vector<glm::vec3>(), std::vector<unsigned int>(), false, glm::vec3(0, 0, 0)) {
    }
    TrackGeometry::TrackGeometry(const std::vector<glm::vec3> &vertices,
                                 const std::vector<glm::vec3> &normals,
                                 const std::vector<glm::vec2> &uvs,
                                 const std::vector<uint32_t> &textureIndices,
                                 const std::vector<uint32_t> &vertexIndices,
                                 const std::vector<glm::vec4> &shadingData,
                                 const std::vector<uint32_t> &debugData,
                                 const glm::vec3 centerPosition) :
        Geometry("TrackMesh", vertices, uvs, normals, vertexIndices, true, centerPosition), m_textureIndices(textureIndices), m_debugData(debugData) {
        // Index Shading data
        for (uint32_t m_vertex_index : vertexIndices) {
            m_shadingData.push_back(shadingData[m_vertex_index]);
        }
    }

    TrackGeometry::TrackGeometry(const std::vector<glm::vec3> &vertices,
                                 const std::vector<glm::vec3> &normals,
                                 const std::vector<glm::vec2> &uvs,
                                 const std::vector<uint32_t> &textureIndices,
                                 const std::vector<uint32_t> &vertexIndices,
                                 const std::vector<glm::vec4> &shadingData,
                                 const glm::vec3 centerPosition) :
        Geometry("TrackMesh", vertices, uvs, normals, vertexIndices, true, centerPosition), m_textureIndices(textureIndices) {
        // Fill the unused buffer with data
        m_debugData.resize(m_textureIndices.size());

        // Index Shading data
        for (const auto &vertexIndex : vertexIndices) {
            m_shadingData.push_back(shadingData[vertexIndex]);
        }
    }

} // namespace LibOpenNFS
