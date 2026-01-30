#pragma once

#include "Common/TextureUtils.h"
#include "Entities/Car.h"
#include "Entities/Track.h"
#include "FCE/FceFile.h"
#include "FEDATA/FedataFile.h"
#include "FRD/FrdFile.h"
#include "Shared/CARP/CarpFile.h"

namespace LibOpenNFS::NFS4 {
    constexpr glm::vec3 TRACK_SCALE_FACTOR(-1, 1, 1);
    constexpr glm::vec3 CAR_SCALE_FACTOR(-1.2, 1.2, 1.2);

    class Loader {
      public:
        static Car LoadCar(std::string const &carBasePath, std::string const &carOutPath, NFSVersion version);
        static Track LoadTrack(std::string const &trackBasePath);

        static FedataFile LoadCarMenuData(std::string const &carBasePath, std::string const &carOutPath, NFSVersion version);

      private:
        static Car::MetaData _ParseAssetData(FceFile const &fceFile, FedataFile const &fedataFile, NFSVersion version);
        static Car::PhysicsData _ParsePhysicsData(Shared::CarpFile const &carpFile);
        static std::map<uint32_t, TrackTextureAsset> _ParseTextures(Track const &track);
        static std::pair<std::vector<TrackBlock>, std::vector<TrackEntity>> _ParseFRDModels(FrdFile const &frdFile, Track &track);
        static std::vector<TrackVRoad> _ParseVirtualRoad(FrdFile const &frdFile);
    };

}; // namespace LibOpenNFS::NFS4
