#pragma once

#include "../../Common/IRawData.h"
#include "../../Shared/VIV/VivArchive.h"

namespace LibOpenNFS::NFS3 {

    static constexpr uint32_t TRACK_NAMES_OFFSET = 0x198;
    static constexpr uint32_t TRACK_COUNT = 9;

    class TextFile final : IRawData {
      public:
        TextFile() = default;

        static bool Load(std::string const &textPath, TextFile &textFile);
        static void Save(std::string const &textPath, TextFile &textFile);

        std::vector<std::string> trackNames;

      private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS::NFS3
