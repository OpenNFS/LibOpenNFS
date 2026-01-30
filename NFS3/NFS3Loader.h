#pragma once

#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#include "../Shared/CAN/CanFile.h"
#include "../Shared/VIV/VivArchive.h"
#include "COL/ColFile.h"
#include "Common/TextureUtils.h"
#include "Entities/Car.h"
#include "Entities/Track.h"
#include "Entities/TrackBlock.h"
#include "Entities/TrackTextureAsset.h"
#include "Entities/TrackVRoad.h"
#include "FCE/FceFile.h"
#include "FEDATA/FedataFile.h"
#include "FRD/FrdFile.h"
#include "SPEEDS/SpeedsFile.h"
#include "Shared/CARP/CarpFile.h"
#include "Shared/HRZ/HrzFile.h"

namespace LibOpenNFS::NFS3 {
    constexpr glm::vec3 TRACK_SCALE_FACTOR(-1, 1, 1);
    constexpr glm::vec3 CAR_SCALE_FACTOR(-1.2, 1.2, 1.2);

    class Loader {
      public:
        static Car LoadCar(std::string const &carBasePath, std::string const &carOutPath);
        static Track LoadTrack(std::string const &trackBasePath);

        static FedataFile LoadCarMenuData(std::string const &carBasePath, std::string const &carOutPath);

      private:
        static Car::MetaData _ParseAssetData(FceFile const &fceFile, FedataFile const &fedataFile);
        static Car::PhysicsData _ParsePhysicsData(Shared::CarpFile const &carpFile);
        static std::map<uint32_t, TrackTextureAsset> _ParseTextures(FrdFile const &frdFile, Track const &track);
        static std::vector<TrackBlock> _ParseFRDModels(FrdFile const &frdFile, Track const &track);
        static std::vector<TrackVRoad> _ParseVirtualRoad(ColFile const &colFile);
        static std::vector<TrackEntity> _ParseCOLModels(ColFile const &colFile, Track const &track, std::vector<TexBlock> &texBlocks);
    };
} // namespace LibOpenNFS::NFS3
