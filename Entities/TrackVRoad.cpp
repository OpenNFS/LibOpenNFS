#include "TrackVRoad.h"

namespace LibOpenNFS {
    TrackVRoad::TrackVRoad(glm::vec3 const position,
                           glm::vec3 const respawn,
                           glm::vec3 const normal,
                           glm::vec3 const forward,
                           glm::vec3 const right,
                           glm::vec3 const leftWall,
                           glm::vec3 const rightWall,
                           uint32_t const unknown) {
        this->position = position;
        this->respawn = respawn;
        this->normal = normal;
        this->forward = forward;
        this->right = right;
        this->leftWall = leftWall;
        this->rightWall = rightWall;
        this->unknown = unknown;
    }
} // namespace LibOpenNFS
