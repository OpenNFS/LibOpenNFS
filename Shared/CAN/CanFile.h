#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS::Shared {
    struct CameraAnimPoint {
        glm::ivec3 pt;
        int16_t od1, od2, od3, od4; // Quaternion
    };

    class CanFile : IRawData {
      public:
        CanFile() = default;

        static bool Load(std::string const &canPath, CanFile &canFile);
        static void Save(std::string const &canPath, CanFile &canFile);

        uint16_t size;
        uint8_t type, struct3D;
        uint16_t animLength, unknown;
        std::vector<CameraAnimPoint> animPoints;

      private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS::Shared
