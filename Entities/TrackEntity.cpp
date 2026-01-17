#include "TrackEntity.h"

namespace LibOpenNFS {
    TrackEntity::TrackEntity(uint32_t const entityID, EntityType const entityType, TrackGeometry const &geometry,
                             std::vector<AnimData> const &animData, uint16_t const animDelay, uint32_t const flags)
        : type(entityType), geometry(geometry), entityID(entityID), flags(flags), hasGeometry(true), animDelay(animDelay),
          animData(animData) {
        this->_SetCollisionParameters();
    }

    TrackEntity::TrackEntity(uint32_t const entityID, EntityType const entityType, TrackGeometry const &geometry, uint32_t const flags)
        : type(entityType), geometry(geometry), entityID(entityID), flags(flags), hasGeometry(true) {
        this->_SetCollisionParameters();
    }

    TrackEntity::TrackEntity(uint32_t const entityID, EntityType const entityType, uint32_t const flags)
        : type(entityType), entityID(entityID), flags(flags) {
        this->_SetCollisionParameters();
    }

    void TrackEntity::_SetCollisionParameters() {
        switch (type) {
        case EntityType::VROAD:
        case EntityType::SOUND:
        case EntityType::LANE:
        case EntityType::GLOBAL:
        case EntityType::CAR:
            collidable = false;
            dynamic = false;
            break;
        case EntityType::ROAD:
            collidable = true;
            dynamic = false;
            break;
        case EntityType::LIGHT: // Light picking
        case EntityType::OBJ_POLY:
        case EntityType::XOBJ:
            collidable = flags & (1 << 5);
            dynamic = false;
            /*switch ((flags >> 4) & 0x7) {
            case 1: // Hometown shack godray
                collideable = false;
                dynamic     = false;
                break;
            case 3: // Hometown start fence
                collideable = true;
                dynamic     = false;
                break;
            case 5: // Roadsign
                collideable = true;
                dynamic     = true;
                break;
            case 6: // Hometown split marker
                collideable = true;
                dynamic     = false;
                break;
            case 7:
                collideable = true;
                dynamic     = false;
                break;
            default:
                collideable = true;
                dynamic     = false;
                break;
            }*/
            break;
        default:
            collidable = false;
            dynamic = false;
            // ASSERT(false, "Entity parameters are unset for %s", get_string(entityType).c_str());
            break;
        }
    }
} // namespace LibOpenNFS
