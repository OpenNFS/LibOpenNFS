#pragma once

#include <string>
#include <magic_enum/magic_enum.hpp>

#include "TrackGeometry.h"

#include <NFS3/Common.h>

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
        TrackEntity(
            uint32_t entityID, EntityType entityType, const TrackGeometry& geometry, std::vector<AnimData> const &animData, uint32_t flags = 0u);
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

        uint16_t nAnimLength, AnimDelay; // JimDiabolo : The bigger the AnimDelay, that slower is the movement
        std::vector<AnimData> animData;

      private:
        void _SetCollisionParameters();
    };
} // namespace LibOpenNFS