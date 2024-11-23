#pragma once

#include "GEO/GeoFile.h"
#include "TRK/TrkFile.h"
#include "COL/ColFile.h"
#include "Entities/Car.h"
#include "Entities/Track.h"
#include "Entities/TrackVRoad.h"
#include "Entities/TrackBlock.h"

namespace LibOpenNFS::NFS2 {
    const float NFS2_SCALE_FACTOR = 1000000.0f;

    template <typename Platform>
    class Loader {
    public:
        /*static Car LoadCar(const std::string &carBasePath, NFSVersion nfsVersion);*/
        static Track LoadTrack(NFSVersion nfsVersion, const std::string &trackBasePath,
                               const std::string &trackOutPath);

    private:
        static Car::MetaData _ParseGEOModels(const GeoFile<Platform> &geoFile);
        static std::vector<LibOpenNFS::TrackBlock>
        _ParseTRKModels(const TrkFile<Platform> &trkFile, ColFile<Platform> &colFile, const Track &track);

        static std::vector<TrackVRoad> _ParseVirtualRoad(ColFile<Platform> &colFile);

        static std::vector<TrackEntity> _ParseCOLModels(ColFile<Platform> &colFile, const Track &track);
    };
} // namespace LibOpenNFS::NFS2
