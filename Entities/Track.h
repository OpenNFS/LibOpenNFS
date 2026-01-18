#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "../Shared/CAN/CanFile.h"
#include "TrackBlock.h"
#include "TrackEntity.h"
#include "TrackTextureAsset.h"
#include "TrackVRoad.h"

namespace LibOpenNFS {
    class Track {
      public:
        Track(NFSVersion _nfsVersion, std::string const &_name, std::string const &_basePath, std::string const &_tag = "");
        Track() = default;

        // Metadata
        NFSVersion nfsVersion{};
        std::string name;
        std::string basePath;
        std::string tag;
        uint32_t nBlocks{0};
        std::vector<Shared::CameraAnimPoint> cameraAnimation;
        std::map<uint32_t, TrackTextureAsset> trackTextureAssets;

        // Geometry
        std::vector<TrackVRoad> virtualRoad;
        std::vector<TrackBlock> trackBlocks;
        std::vector<TrackEntity> globalObjects;
    };
} // namespace LibOpenNFS
