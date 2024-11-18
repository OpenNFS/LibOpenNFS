#pragma once

#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include <bitset>

#include "Common/TextureUtils.h"
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
        static Track LoadTrack(const std::string &trackBasePath, NFSVersion nfsVersion);

    private:
        static Car::MetaData _ParseGEOModels(const LibOpenNFS::NFS2::GeoFile<Platform> &geoFile);
        static std::vector<LibOpenNFS::TrackBlock>
        _ParseTRKModels(const LibOpenNFS::NFS2::TrkFile<Platform> &trkFile, LibOpenNFS::NFS2::ColFile<Platform> &colFile, const Track &track);
        static std::vector<TrackVRoad> _ParseVirtualRoad(LibOpenNFS::NFS2::ColFile<Platform> &colFile);
        static std::vector<TrackEntity> _ParseCOLModels(LibOpenNFS::NFS2::ColFile<Platform> &colFile, const Track &track);
    };
} // namespace LibOpenNFS::NFS2
