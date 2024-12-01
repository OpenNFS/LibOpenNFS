#include "TrkFile.h"

#include <cstring>

#include "Common/Logging.h"

using namespace LibOpenNFS::NFS2;

template <typename Platform>
bool TrkFile<Platform>::Load(std::string const &trkPath, TrkFile &trkFile, NFSVersion version) {
    LogInfo("Loading TRK File located at %s", trkPath.c_str());
    std::ifstream trk(trkPath, std::ios::in | std::ios::binary);
    trkFile.version = version;

    bool const loadStatus{trkFile._SerializeIn(trk)};
    trk.close();

    return loadStatus;
}

template <typename Platform> void TrkFile<Platform>::Save(std::string const &trkPath, TrkFile &trkFile) {
    LogInfo("Saving TRK File to %s", trkPath.c_str());
    std::ofstream trk(trkPath, std::ios::out | std::ios::binary);
    trkFile._SerializeOut(trk);
}

template <typename Platform> bool TrkFile<Platform>::_SerializeIn(std::ifstream &ifstream) {
    // Check we're in a valid TRK file
    onfs_check(safe_read(ifstream, header, HEADER_LENGTH));

    // Header should contain TRAC
    if (memcmp(header, "TRAC", sizeof(header)) != 0) {
        LogWarning("Invalid TRK Header");
        return false;
    }

    // Unknown header data
    onfs_check(safe_read(ifstream, unknownHeader, UNKNOWN_HEADER_LENGTH * sizeof(uint32_t)));

    // Basic Track data
    onfs_check(safe_read(ifstream, nSuperBlocks));
    onfs_check(safe_read(ifstream, nBlocks));

    // Offsets of Superblocks in TRK file
    superBlockOffsets.resize(nSuperBlocks);
    onfs_check(safe_read(ifstream, superBlockOffsets));

    // Reference coordinates for each block
    blockReferenceCoords.resize(nBlocks);
    onfs_check(safe_read(ifstream, blockReferenceCoords));

    // Go read the superblocks in
    for (uint32_t superBlockIdx = 0; superBlockIdx < nSuperBlocks; ++superBlockIdx) {
        LogDebug("SuperBlock %d of %d", superBlockIdx + 1, nSuperBlocks);
        // Jump to the super block
        ifstream.seekg(superBlockOffsets[superBlockIdx], std::ios_base::beg);
        superBlocks.push_back(SuperBlock<Platform>(ifstream, this->version));
    }

    return true;
}

template <typename Platform> void TrkFile<Platform>::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "TRK output serialization is not currently implemented");
}

template class LibOpenNFS::NFS2::TrkFile<PS1>;
template class LibOpenNFS::NFS2::TrkFile<PC>;