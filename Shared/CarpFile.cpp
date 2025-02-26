#include "CarpFile.h"

#include "Common/Logging.h"
#include "Common/TextureUtils.h"

namespace LibOpenNFS::Shared {
    bool CarpFile::Load(std::string const &carpPath, CarpFile &carpFile) {
        LogInfo("Loading carp.txt File located at %s", carpPath.c_str());
        std::ifstream carp(carpPath, std::ios::in | std::ios::binary);

        bool const loadStatus{carpFile._SerializeIn(carp)};
        carp.close();

        return loadStatus;
    }

    void CarpFile::Save(std::string const &carpPath, CarpFile &carpFile) {
        LogInfo("Saving carp.txt File to %s", carpPath.c_str());
        std::ofstream carp(carpPath, std::ios::out | std::ios::binary);
        carpFile._SerializeOut(carp);
    }

    bool CarpFile::_SerializeIn(std::ifstream &ifstream) {       
        std::string str, data;

        while (std::getline(ifstream, str)) {
            int entry = std::atoi(str.substr(str.rfind('(') + 1, str.rfind(')') -1 ).c_str());  // The entry number is between () in the string
            std::getline(ifstream, data);
            switch (entry)
            {
                case 0:
                    if (data.empty()) {
                        return false;
                    }
                    
                    break;
            default:
                break;
            }
        }

        return true;
    }

    void CarpFile::_SerializeOut(std::ofstream &ofstream) {
        ASSERT(false, "HRZ Output serialization is not implemented yet");
    }
} // namespace LibOpenNFS::Shared
