#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace LibOpenNFS {
    class Geometry {
      public:
        Geometry() = default;
        Geometry(std::string name, std::vector<glm::vec3> const &vertices, std::vector<glm::vec2> const &uvs,
                 std::vector<glm::vec3> const &normals, std::vector<uint32_t> const &vertexIndices, bool removeVertexIndexing,
                 glm::vec3 const &centerPosition);
        virtual ~Geometry() = default;
        std::string name;
        std::vector<glm::vec3> m_vertices;
        std::vector<glm::vec3> m_normals;
        std::vector<glm::vec2> m_uvs;
        std::vector<uint32_t> m_vertexIndices;

        glm::vec3 position{};
        glm::vec3 initialPosition{};
        glm::vec3 orientation_vec{};
        glm::quat orientation{};
    };
} // namespace LibOpenNFS
