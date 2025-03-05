#pragma once

#include "Common/TextureUtils.h"
#include "Entities/Car.h"
#include "Entities/Track.h"
#include "FCE/FceFile.h"
#include "FEDATA/FedataFile.h"
#include "FRD/FrdFile.h"

namespace LibOpenNFS::NFS4 {
    constexpr glm::vec3 NFS4_SCALE_FACTOR(-0.1, 0.1, 0.1f);
    class Loader {
      public:
        static Car LoadCar(std::string const &carBasePath, std::string const &carOutPath, NFSVersion version);
        static Track LoadTrack(std::string const &trackBasePath, std::string const &trackOutPath);

      private:
        static Car::MetaData _ParseAssetData(FceFile const &fceFile, FedataFile const &fedataFile, NFSVersion version);
        static std::map<uint32_t, TrackTextureAsset> _ParseTextures(Track const &track, std::string const &trackOutPath);
        static std::pair<std::vector<TrackBlock>, std::vector<TrackEntity>> _ParseFRDModels(FrdFile const &frdFile, Track &track);
        static std::vector<TrackVRoad> _ParseVirtualRoad(FrdFile const &frdFile);
    };

}; // namespace LibOpenNFS::NFS4
