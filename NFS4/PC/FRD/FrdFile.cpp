#include "FrdFile.h"

#include "Common/Logging.h"

namespace LibOpenNFS::NFS4 {
    bool FrdFile::Load(std::string const &frdPath, FrdFile &frdFile) {
        LogInfo("Loading FRD File located at %s", frdPath.c_str());
        std::ifstream frd(frdPath, std::ios::in | std::ios::binary);

        bool const loadStatus{frdFile._SerializeIn(frd)};
        frd.close();

        return loadStatus;
    }

    void FrdFile::Save(std::string const &frdPath, FrdFile &frdFile) {
        LogInfo("Saving FRD File to %s", frdPath.c_str());
        std::ofstream frd(frdPath, std::ios::out | std::ios::binary);
        frdFile._SerializeOut(frd);
    }

    bool FrdFile::_SerializeIn(std::ifstream &ifstream) {
        onfs_check(safe_read(ifstream, header));
        onfs_check(safe_read(ifstream, nBlocks));
        ++nBlocks;
        onfs_check(nBlocks > 1 && nBlocks <= 500);
        onfs_check(safe_read(ifstream, numVRoad));
        onfs_check(nBlocks > 1 && nBlocks <= 500);
        onfs_check(((numVRoad + 7) / 8) == nBlocks);

        for (uint32_t i = 0; i < numVRoad; i++) {
            vroadBlocks.emplace_back(ifstream);
        }
        for (uint32_t blockIdx = 0; blockIdx < nBlocks; blockIdx++) {
            trackBlockHeaders.emplace_back(ifstream);
        }
        for (uint32_t blockIdx = 0; blockIdx < nBlocks; blockIdx++) {
            trackBlocks.emplace_back(trackBlockHeaders.at(blockIdx), ifstream);
        }
        for (uint32_t globalIdx = 0; globalIdx < 2; ++globalIdx) {
            onfs_check(safe_read(ifstream, nGlobalObjects[globalIdx]));
            globalObjects.emplace_back(nGlobalObjects[globalIdx], ifstream);
        }

        return true;
    }

    void FrdFile::_SerializeOut(std::ofstream &ofstream) {
        ASSERT(false, "FrdFile output serialization is not currently implemented");
    }
} // namespace LibOpenNFS::NFS4
