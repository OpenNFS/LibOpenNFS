#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"
#include "GeoBlock.h"

namespace LibOpenNFS::NFS2 {
    template <typename Platform> class GeoFile : IRawData {
      public:
        GeoFile() = default;

        static bool Load(std::string const &geoPath, GeoFile &geoFile);
        static void Save(std::string const &geoPath, GeoFile &geoFile);

        Platform::HEADER header;
        std::vector<GeoBlock<Platform>> blocks;

      private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS::NFS2