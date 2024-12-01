#include "TrackSound.h"

namespace LibOpenNFS {
    TrackSound::TrackSound(uint32_t const entityID, glm::vec3 const position, uint32_t const type)
        : TrackEntity(entityID, EntityType::SOUND, 0u) {
        this->position = position;
        this->type = type;
    }
} // namespace LibOpenNFS