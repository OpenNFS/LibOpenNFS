#include "BnkFile.h"

#include <cstring>

using namespace LibOpenNFS::NFS3;

bool BnkFile::Load(std::string const &bnkPath, BnkFile &bnkFile) {
    LogInfo("Loading BNK File located at %s", bnkPath.c_str());
    std::ifstream bnk(bnkPath, std::ios::in | std::ios::binary);

    bool const loadStatus = bnkFile._SerializeIn(bnk);
    bnk.close();

    return loadStatus;
}

void BnkFile::Save(std::string const &bnkPath, BnkFile &bnkFile) {
    LogInfo("Saving BNK File to %s", bnkPath.c_str());
    std::ofstream bnk(bnkPath, std::ios::out | std::ios::binary);
    bnkFile._SerializeOut(bnk);
}

bool BnkFile::_SerializeIn(std::ifstream &ifstream) {
    // Read in the BNK file header
    onfs_check(safe_read(ifstream, header, 4));
    if (memcmp(header, "BNKl", sizeof(char)) != 0) {
        LogWarning("Invalid BNK file");
        return false;
    }

    onfs_check(safe_read(ifstream, &version));
    onfs_check(safe_read(ifstream, &numSounds));
    onfs_check(safe_read(ifstream, &firstSoundOffset));

    soundOffsets.resize(numSounds);
    onfs_check(safe_read(ifstream, soundOffsets);

    return true;
}

void BnkFile::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "COL output serialization is not currently implemented");
}
