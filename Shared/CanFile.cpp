#include "CanFile.h"

#include "Common/Logging.h"

namespace LibOpenNFS::Shared {
    bool CanFile::Load(const std::string &canPath, CanFile &canFile) {
        LogInfo("Loading CAN File located at %s", canPath.c_str());
        std::ifstream can(canPath, std::ios::in | std::ios::binary);

        bool const loadStatus{canFile._SerializeIn(can)};
        can.close();

        return loadStatus;
    }

    void CanFile::Save(const std::string &canPath, CanFile &canFile) {
        LogInfo("Saving CAN File to %s", canPath.c_str());
        std::ofstream can(canPath, std::ios::out | std::ios::binary);
        canFile._SerializeOut(can);
    }

    bool CanFile::_SerializeIn(std::ifstream &ifstream) {
        onfs_check(safe_read(ifstream, size));
        onfs_check(safe_read(ifstream, type));
        onfs_check(safe_read(ifstream, struct3D));
        onfs_check(safe_read(ifstream, animLength));
        onfs_check(safe_read(ifstream, unknown));

        animPoints.resize(animLength);
        onfs_check(safe_read(ifstream, animPoints));

        return true;
    }

    void CanFile::_SerializeOut(std::ofstream &ofstream) {
        ofstream.write((char *) &size, sizeof(uint16_t));
        ofstream.write((char *) &type, sizeof(uint8_t));
        ofstream.write((char *) &struct3D, sizeof(uint8_t));
        ofstream.write((char *) &animLength, sizeof(uint16_t));
        ofstream.write((char *) &unknown, sizeof(uint16_t));
        ofstream.write((char *) animPoints.data(), sizeof(CameraAnimPoint) * animLength);
        ofstream.close();
    }
}
