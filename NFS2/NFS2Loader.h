#pragma once

#include "COL/ColFile.h"
#include "Entities/Car.h"
#include "Entities/Track.h"
#include "Entities/TrackBlock.h"
#include "Entities/TrackVRoad.h"
#include "GEO/GeoFile.h"
#include "TRK/TrkFile.h"

namespace LibOpenNFS::NFS2 {
    constexpr glm::vec3 SCALE_FACTOR(-0.000001, 0.000001, 0.000001f);
    constexpr float CAR_SCALE_FACTOR = 2000.f;

    template <typename Platform> class Loader {
      public:
        static Car LoadCar(std::string const &carBasePath, std::string const &carOutPath, NFSVersion nfsVersion);
        static Track LoadTrack(NFSVersion nfsVersion, std::string const &trackBasePath, std::string const &trackOutPath);

      private:
        static Car::MetaData _ParseGEOModels(GeoFile<Platform> const &geoFile);
        static std::map<uint32_t, TrackTextureAsset> _ParseTextures(Track const &track, std::string const &trackOutPath);
        static std::vector<LibOpenNFS::TrackBlock> _ParseTRKModels(TrkFile<Platform> const &trkFile, ColFile<Platform> &colFile,
                                                                   Track const &track);
        static std::vector<TrackVRoad> _ParseVirtualRoad(ColFile<Platform> &colFile);
        static std::vector<TrackEntity> _ParseCOLModels(ColFile<Platform> &colFile, Track const &track);
    };
} // namespace LibOpenNFS::NFS2
