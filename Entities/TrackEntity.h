#pragma once

#include "TrackGeometry.h"

#include "../Shared/AnimKeyframe.h"

namespace LibOpenNFS {
    enum class EntityType {
        XOBJ,
        OBJ_POLY,
        LANE,
        SOUND,
        LIGHT,
        ROAD,
        GLOBAL,
        CAR,
        VROAD
    };

    class TrackEntity {
      public:
        TrackEntity(uint32_t entityID, EntityType entityType, TrackGeometry const &geometry, std::vector<AnimKeyframe> const &AnimKeyframes,
                    uint16_t animDelay, uint32_t flags = 0u);
        TrackEntity(uint32_t entityID, EntityType entityType, TrackGeometry const &geometry, uint32_t flags = 0u);
        TrackEntity(uint32_t entityID, EntityType entityType, uint32_t flags = 0u);
        virtual ~TrackEntity() = default;

        EntityType type;
        TrackGeometry geometry;
        uint32_t entityID{0};
        uint32_t flags{0};
        bool hasGeometry{false};
        bool collidable{false};
        bool dynamic{false};

        uint16_t animDelay;
        std::vector<AnimKeyframe> animKeyframes;

      private:
        void _SetCollisionParameters();
    };
} // namespace LibOpenNFS