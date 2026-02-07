#pragma once

#include <map>

#include "../../Common/IRawData.h"
#include "../../Shared/VIV/VivArchive.h"

namespace LibOpenNFS::NFS4 {

  static std::vector<uint32_t> TRACK_NAME_OFFSETS{
    0x4F4,  // Aquatica
    0x4E4,  // Atlantica
    0x4BC,  // Dolphin Cove
    0x4EC,  // Country Woods
    0x4FC,  // Empire City
    0x4C4,  // Route Adonf
    0x4B8,  // Landstrasse
    0x4D0,  // Raceway
    0x4D4,  // Raceway 2
    0x4D8,  // Raceway 3
    0x4B4,  // Celtic Ruins
    0x4DC,  // Hometown
    0x4F0,  // Lost Canyons
    0x4C0,  // Kindiak Park
    0x4E0,  // Redrock Ridge
    0x4E8,  // Rocky Pass
    0x4CC,  // Snowy Ridge
    0x4F8,  // Summit
    0x4C8,  // Durham Road
  };

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
} // namespace LibOpenNFS::NFS4
