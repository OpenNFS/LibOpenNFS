#pragma once

#include "Common/TextureUtils.h"
#include "Entities/Car.h"
#include "Entities/Track.h"
#include "FCE/FceFile.h"
#include "FEDATA/FedataFile.h"
#include "FRD/FrdFile.h"

namespace LibOpenNFS::NFS4 {
    glm::vec3 const NFS4_SCALE_FACTOR(-0.1, 0.1, 0.1f);
    class Loader {
      public:
        static Car LoadCar(std::string const &carBasePath, std::string const &carOutPath, NFSVersion version);
        static Track LoadTrack(const std::string &trackBasePath, const std::string &trackOutPath);

      private:
        static Car::MetaData _ParseAssetData(FceFile const &fceFile, FedataFile const &fedataFile, NFSVersion version);
        //static std::map<uint32_t, TrackTextureAsset> _ParseTextures(const FrdFile &frdFile, const Track &track, const std::string &trackOutPath);
        //static std::vector<TrackBlock> _ParseTRKModels(const FrdFile &frdFile, const Track &track);
        //static std::vector<TrackVRoad> _ParseVirtualRoad(const ColFile &colFile);
        //static std::vector<TrackEntity> _ParseCOLModels(const ColFile &colFile, const Track &track, std::vector<TexBlock> &texBlocks);
    };

}; // namespace LibOpenNFS::NFS4
