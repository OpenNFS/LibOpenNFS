#include "GeoFile.h"

#include "Common/Logging.h"

namespace LibOpenNFS::NFS2 {
    template <typename Platform> bool GeoFile<Platform>::Load(std::string const &geoPath, GeoFile &geoFile) {
        LogInfo("Loading GEO File located at %s", geoPath.c_str());
        std::ifstream geo(geoPath, std::ios::in | std::ios::binary);

        bool const loadStatus = geoFile._SerializeIn(geo);
        geo.close();

        return loadStatus;
    }

    template <typename Platform> void GeoFile<Platform>::Save(std::string const &geoPath, GeoFile &geoFile) {
        LogInfo("Saving FCE File to %s", geoPath.c_str());
        std::ofstream geo(geoPath, std::ios::out | std::ios::binary);
        geoFile._SerializeOut(geo);
    }

    template <typename Platform> bool GeoFile<Platform>::_SerializeIn(std::ifstream &ifstream) {
        onfs_check(safe_read(ifstream, header, sizeof(typename Platform::HEADER)));
        for (uint32_t partIdx {0}; partIdx < Platform::PART_NAMES.size(); ++partIdx) {
            blocks.emplace_back(ifstream, partIdx);
            // TODO: This is a hack, something in the GeoBlock serialisation is off for PS1
            if (std::is_same<PS1, Platform>() && partIdx > 20) {
                break;
            }
        }

        return true;
    }

    template <typename Platform> void GeoFile<Platform>::_SerializeOut(std::ofstream &ofstream) {
        ASSERT(false, "GEO output serialization is not currently implemented");
    }

    template class GeoFile<PS1>;
    template class GeoFile<PC>;
} // namespace LibOpenNFS::NFS2