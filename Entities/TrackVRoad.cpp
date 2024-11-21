#include "TrackVRoad.h"

namespace LibOpenNFS {
    TrackVRoad::TrackVRoad(const glm::vec3 position, const glm::vec3 respawn, const glm::vec3 normal, const glm::vec3 forward, const glm::vec3 right, const glm::vec3 leftWall, const glm::vec3 rightWall, const uint32_t unknown) {
        this->position  = position;
        this->respawn   = respawn;
        this->normal    = normal;
        this->forward   = forward;
        this->right     = right;
        this->leftWall  = leftWall;
        this->rightWall = rightWall;
        this->unknown   = unknown;
    }
} // namespace LibOpenNFS
