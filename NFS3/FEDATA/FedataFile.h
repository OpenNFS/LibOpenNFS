#pragma once

#include "../../Common/IRawData.h"
#include "../../Shared/VIV/VivArchive.h"

namespace LibOpenNFS::NFS3 {

    static constexpr uint32_t ID_LENGTH = 4;
    static constexpr uint32_t STATS_OFFSET = 40;
    static constexpr uint32_t STRING_OFFSET_OFFSET = 47;
    static constexpr uint32_t EXPECTED_FLAG_COUNT = 9;
    static constexpr uint32_t EXPECTED_STRING_ENTRIES = 40;
    static constexpr uint32_t HISTORY_COUNT = 8;
    static constexpr uint32_t COLOR_COUNT = 10;

    class FedataFile final : IRawData {
      public:
        enum SeatPosition {
            LEFT=0,
            RIGHT=1,
            Center=2
        };

        FedataFile() = default;

        static bool Load(std::string const &fedataPath, FedataFile &fedataFile);
        static void Save(std::string const &fedataPath, FedataFile &fedataFile);

        std::string id;

        // General data
        bool isBonus = false;
        bool isAvailableToAi = false;
        uint16_t vehicleClass = 2;  // 2 is the slowest class, 0 the fastest
        uint16_t unknown1 = 3;  // No idea what this is, but it is always 3
        bool isDlcCar = 0;
        bool isPolice = false;
        SeatPosition seatPosition = LEFT;
        uint16_t unknown2 = 1;  // Set to 0 for merc, elni, peln, knoc and lcop. Set to 1 for everything else
        uint16_t unknown3 = 0;  // This value is different per car, but we have no clue what it means
        uint16_t serial = 0;

        // Compare values
        uint8_t acceleration = 0;
        uint8_t topSpeed = 0;
        uint8_t handling = 0;
        uint8_t breaking = 0;
        uint8_t unknownStat = 5;  // This is always 5 for some reason

        // Text translation values
        std::string manufacturer;
        std::string model;
        std::string carName;
        std::string price;
        std::string status;
        std::string weight;
        std::string weightDistribution;
        std::string length;
        std::string width;
        std::string height;
        std::string engine;
        std::string displacement;
        std::string horsePower;
        std::string torque;
        std::string maximumRpm;
        std::string brakes;
        std::string tires;
        std::string topSpeedText;
        std::string zeroToSixty;
        std::string zeroToOneHundred;
        std::string transmission;
        std::string gearbox;

        std::vector<std::string> history;
        std::vector<std::string> primaryColourNames;

      private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;

        void _convertToUtf8(std::string &string);
    };
} // namespace LibOpenNFS::NFS3
