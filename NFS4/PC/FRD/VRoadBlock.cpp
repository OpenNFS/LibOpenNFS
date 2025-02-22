#include "VRoadBlock.h"

namespace LibOpenNFS::NFS4{

VRoadBlock::VRoadBlock(std::ifstream &frd) {
    ASSERT(this->VRoadBlock::_SerializeIn(frd), "Failed to serialize VRoadBlock from file stream");
}

bool VRoadBlock::_SerializeIn(std::ifstream &frd) {
    onfs_check(safe_read(frd, refPt));
    onfs_check(safe_read(frd, normal));
    onfs_check(safe_read(frd, forward));
    onfs_check(safe_read(frd, right));
    onfs_check(safe_read(frd, leftWall));
    onfs_check(safe_read(frd, rightWall));
    onfs_check(safe_read(frd, unknown1));
    onfs_check(safe_read(frd, unknown2));
    return true;
}

void VRoadBlock::_SerializeOut(std::ofstream &frd) {
    ASSERT(false, "VRoadBlock output serialization is not currently implemented");
}
}
