#include "CarGeometry.h"

namespace LibOpenNFS {
    // TODO: These all kinda do the same thing, so kill the extras
    CarGeometry::CarGeometry(std::string const &name,
                             std::vector<glm::vec3> const &verts,
                             std::vector<glm::vec2> const &uvs,
                             std::vector<uint32_t> const &texture_indices,
                             std::vector<uint32_t> const &test,
                             std::vector<glm::vec3> const &norms,
                             std::vector<uint32_t> const &indices,
                             glm::vec3 const &center_position)
        : Geometry(name, verts, uvs, norms, indices, true, center_position) {
        m_texture_indices = texture_indices;
        isMultiTextured = true;
        // Fill the unused buffer with data
        m_polygon_flags = test;
        m_normals.clear();
        for (auto const &vert_index : m_vertexIndices) {
            m_normals.push_back(norms[vert_index]);
        }
    }

    CarGeometry::CarGeometry(std::string const &name,
                             std::vector<glm::vec3> const &verts,
                             std::vector<glm::vec2> const &uvs,
                             std::vector<uint32_t> const &texture_indices,
                             std::vector<glm::vec3> const &norms,
                             std::vector<uint32_t> const &indices,
                             glm::vec3 const &center_position)
        : Geometry(name, verts, uvs, norms, indices, true, center_position) {
        m_texture_indices = texture_indices;
        isMultiTextured = true;
        m_polygon_flags.resize(m_texture_indices.size());
        m_normals = norms;
    }

    CarGeometry::CarGeometry(std::string const &name,
                             std::vector<glm::vec3> const &verts,
                             std::vector<glm::vec2> const &uvs,
                             std::vector<glm::vec3> const &norms,
                             std::vector<uint32_t> const &indices,
                             std::vector<uint32_t> const &poly_flags,
                             glm::vec3 const &center_position)
        : Geometry(name, verts, uvs, norms, indices, true, center_position) {
        m_polygon_flags = poly_flags;
        m_texture_indices.resize(m_vertexIndices.size());
        m_normals.clear();
        for (auto const &vertex_index : m_vertexIndices) {
            m_normals.push_back(norms[vertex_index]);
        }
    }

    CarGeometry::CarGeometry(std::string const &name,
                             std::vector<glm::vec3> const &verts,
                             std::vector<glm::vec2> const &uvs,
                             std::vector<glm::vec3> const &norms,
                             std::vector<uint32_t> const &indices,
                             glm::vec3 const &center_position)
        : Geometry(name, verts, uvs, norms, indices, false, center_position) {
        m_texture_indices.resize(m_normals.size());
        m_polygon_flags.resize(m_texture_indices.size());
        m_normals.clear();
        for (auto const &vertex_index : m_vertexIndices) {
            m_normals.push_back(norms[vertex_index]);
        }
    }
} // namespace LibOpenNFS