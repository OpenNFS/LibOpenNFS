#include "BaseLight.h"

namespace LibOpenNFS {
    BaseLight::BaseLight(uint32_t const entityID,
                         uint32_t const flags,
                         LightType const type,
                         glm::vec3 const position,
                         glm::vec4 const colour)
        : TrackEntity(entityID, EntityType::LIGHT, flags), type(type), position(position), colour(colour),
          attenuation(2.0f, 0.1f, 0.1f) {
    }
} // namespace LibOpenNFS
