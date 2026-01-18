#pragma once

#include "Common/IRawData.h"

namespace LibOpenNFS::Shared {
    struct VivEntry {
        char filename[100];
        std::vector<uint8_t> data;
    };

    class VivArchive : IRawData {
      public:
        VivArchive() = default;
        static bool Load(std::string const &vivPath, VivArchive &vivFile);
        static void Save(std::string const &vivPath, VivArchive &vivFile);
        static bool Extract(std::string const &outPath, VivArchive &vivFile);

        char vivHeader[4];
        uint32_t vivSize;
        uint32_t nFiles;
        uint32_t startPos;
        std::vector<VivEntry> files;

      private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS::Shared