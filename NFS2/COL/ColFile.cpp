#include "ColFile.h"

#include <cstring>

#include "Common/Logging.h"

using namespace LibOpenNFS::NFS2;

template <typename Platform> bool ColFile<Platform>::Load(std::string const &colPath, ColFile &colFile, NFSVersion version) {
    LogInfo("Loading COL File located at %s", colPath.c_str());
    std::ifstream col(colPath, std::ios::in | std::ios::binary);
    colFile.version = version;

    bool const loadStatus = colFile._SerializeIn(col);
    col.close();

    return loadStatus;
}

template <typename Platform> void ColFile<Platform>::Save(std::string const &colPath, ColFile &colFile) {
    LogInfo("Saving COL File to %s", colPath.c_str());
    std::ofstream col(colPath, std::ios::out | std::ios::binary);
    colFile._SerializeOut(col);
}

template <typename Platform> bool ColFile<Platform>::_SerializeIn(std::ifstream &ifstream) {
    // Check we're in a valid TRK file
    onfs_check(safe_read(ifstream, header, HEADER_LENGTH));
    if (memcmp(header, "COLL", sizeof(header)) != 0)
        return false;

    onfs_check(safe_read(ifstream, colVersion));
    if (colVersion != 11)
        return false;

    onfs_check(safe_read(ifstream, size));
    onfs_check(safe_read(ifstream, nExtraBlocks));

    extraBlockOffsets.resize(nExtraBlocks);
    onfs_check(safe_read(ifstream, extraBlockOffsets));

    LogInfo("Version: %d nExtraBlocks: %d", colVersion, nExtraBlocks);
    LogDebug("Parsing COL Extrablocks");

    for (uint32_t extraBlockIdx = 0; extraBlockIdx < nExtraBlocks; ++extraBlockIdx) {
        ifstream.seekg(16 + extraBlockOffsets[extraBlockIdx], std::ios_base::beg);
        extraObjectBlocks.push_back(ExtraObjectBlock<Platform>(ifstream, this->version));
        // Map the the block type to the vector index, gross, original ordering is then maintained for output
        // serialisation
        extraObjectBlockMap[(ExtraBlockID)extraObjectBlocks.back().id] = extraBlockIdx;
    }

    return true;
}

template <typename Platform> void ColFile<Platform>::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "COL output serialization is not currently implemented");
}
template <typename Platform> ExtraObjectBlock<Platform> ColFile<Platform>::GetExtraObjectBlock(ExtraBlockID const eBlockType) {
    return extraObjectBlocks[extraObjectBlockMap[eBlockType]];
}
template <typename Platform> bool ColFile<Platform>::IsBlockPresent(ExtraBlockID const eBlockType) const {
    return extraObjectBlockMap.contains(eBlockType);
}

template class LibOpenNFS::NFS2::ColFile<PS1>;
template class LibOpenNFS::NFS2::ColFile<PC>;