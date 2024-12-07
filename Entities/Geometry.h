#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace LibOpenNFS {
    class Geometry {
    public:
        Geometry() = default;
        Geometry(std::string name, const std::vector<glm::vec3> &vertices, const std::vector<glm::vec2> &uvs, const std::vector<glm::vec3> &normals,
                 const std::vector<uint32_t> &vertexIndices, bool removeVertexIndexing, const glm::vec3 &centerPosition);
        virtual ~Geometry() = default;
        std::string m_name;
        std::vector<glm::vec3> m_vertices;
        std::vector<glm::vec3> m_normals;
        std::vector<glm::vec2> m_uvs;
        std::vector<uint32_t> m_vertexIndices;

        glm::vec3 position {};
        glm::vec3 initialPosition {};
        glm::vec3 orientation_vec {};
        glm::quat orientation {};
    };
} // namespace LibOpenNFS
