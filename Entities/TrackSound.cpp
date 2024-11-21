#include "TrackSound.h"

namespace LibOpenNFS {
    TrackSound::TrackSound(const uint32_t entityID, const glm::vec3 position, const uint32_t type) : TrackEntity(entityID, EntityType::SOUND, 0u) {
        this->position = position;
        this->type     = type;
    }
} // namespace LibOpenNFS