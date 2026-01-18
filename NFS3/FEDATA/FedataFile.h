#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS::NFS3 {

    static constexpr uint32_t COLOUR_TABLE_OFFSET = 0xA7;
    static constexpr uint32_t MENU_NAME_FILEPOS_OFFSET = 0x37;

    class FedataFile final : IRawData {
      public:
        FedataFile() = default;

        static bool Load(std::string const &fedataPath, FedataFile &fedataFile, uint8_t nPriColours);
        static void Save(std::string const &fedataPath, FedataFile &fedataFile);

        std::string menuName;
        std::vector<std::string> primaryColourNames;

      private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;

        uint8_t m_nPriColours;
    };
} // namespace LibOpenNFS::NFS3
