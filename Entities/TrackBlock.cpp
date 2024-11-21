#include "TrackBlock.h"

namespace LibOpenNFS {
    TrackBlock::TrackBlock(const uint32_t id, const glm::vec3 position, const uint32_t virtualRoadStartIndex, const uint32_t nVirtualRoadPositions, const std::vector<uint32_t> &neighbourIds) {
        this->id                    = id;
        this->position              = position;
        this->virtualRoadStartIndex = virtualRoadStartIndex;
        this->nVirtualRoadPositions = nVirtualRoadPositions;
        this->neighbourIds          = neighbourIds;
    }
} // namespace LibOpenNFS
