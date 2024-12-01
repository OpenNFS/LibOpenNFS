#include "TrackBlock.h"

namespace LibOpenNFS {
    TrackBlock::TrackBlock(uint32_t const id,
                           glm::vec3 const position,
                           uint32_t const virtualRoadStartIndex,
                           uint32_t const nVirtualRoadPositions,
                           std::vector<uint32_t> const &neighbourIds) {
        this->id = id;
        this->position = position;
        this->virtualRoadStartIndex = virtualRoadStartIndex;
        this->nVirtualRoadPositions = nVirtualRoadPositions;
        this->neighbourIds = neighbourIds;
    }
} // namespace LibOpenNFS
