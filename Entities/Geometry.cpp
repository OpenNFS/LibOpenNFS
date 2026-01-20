#include "Geometry.h"

#include <utility>

namespace LibOpenNFS {
    Geometry::Geometry(std::string name, std::vector<glm::vec3> const &vertices, std::vector<glm::vec2> const &uvs,
                       std::vector<glm::vec3> const &normals, std::vector<uint32_t> const &vertexIndices, bool const removeVertexIndexing,
                       glm::vec3 const &centerPosition)
        : name(std::move(name)), m_normals(normals), m_uvs(uvs), m_vertexIndices(vertexIndices) {
        if (removeVertexIndexing) {
            for (auto const &vertex_index : m_vertexIndices) {
                m_vertices.push_back(vertices[vertex_index]);
            }
        } else {
            m_vertices = vertices;
        }

        position = centerPosition;
        initialPosition = centerPosition;
        orientation_vec = glm::vec3(0, 0, 0);
        orientation = glm::normalize(glm::quat(orientation_vec));
    }
} // namespace LibOpenNFS
