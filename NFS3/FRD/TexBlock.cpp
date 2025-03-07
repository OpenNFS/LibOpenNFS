#include "TexBlock.h"

using namespace LibOpenNFS::NFS3;

TexBlock::TexBlock(std::ifstream &frd) {
    ASSERT(this->_SerializeIn(frd), "Failed to serialize TextureBlock from file stream");
}

bool TexBlock::_SerializeIn(std::ifstream &ifstream) {
    onfs_check(safe_read(ifstream, width));
    onfs_check(safe_read(ifstream, height));
    onfs_check(safe_read(ifstream, unknown1));
    onfs_check(safe_read(ifstream, corners));
    onfs_check(safe_read(ifstream, unknown2));
    onfs_check(safe_read(ifstream, isLane));
    onfs_check(safe_read(ifstream, qfsIndex));

    return true;
}

void TexBlock::_SerializeOut(std::ofstream &ofstream) {
    // TODO: Do I need to align here?
    ofstream.write((char *)&width, sizeof(uint16_t));
    ofstream.write((char *)&height, sizeof(uint16_t));
    ofstream.write((char *)&unknown1, sizeof(uint32_t));
    ofstream.write((char *)&corners, sizeof(float) * 8);
    ofstream.write((char *)&unknown2, sizeof(uint32_t));
    ofstream.write((char *)&isLane, sizeof(bool));
    ofstream.write((char *)&qfsIndex, sizeof(uint16_t));
}

std::vector<glm::vec2> TexBlock::GetUVs() {
    return {{corners[0], corners[1]}, {corners[2], corners[3]}, {corners[4], corners[5]},
            {corners[0], corners[1]}, {corners[4], corners[5]}, {corners[6], corners[7]}};
}
