#include "AnimBlock.h"


namespace LibOpenNFS::NFS4 {

    AnimBlock::AnimBlock(XObjHeader const &_header, std::ifstream &frd) : BaseObjectBlock::BaseObjectBlock(_header, frd) {
        ASSERT(this->AnimBlock::_SerializeIn(frd), "Failed to serialize AnimBlock from file stream");
        ASSERT(this->BaseObjectBlock::_SerializeIn(frd), "Failed to serialize BaseObjectBlock from file stream");
    }

    bool AnimBlock::_SerializeIn(std::ifstream &frd) {
        onfs_check(safe_read(frd, unknown));
        onfs_check(safe_read(frd, type));
        onfs_check(safe_read(frd, id));
        onfs_check(safe_read(frd, nKeyframes));
        onfs_check(safe_read(frd, delay));
        keyframes.resize(nKeyframes);
        onfs_check(safe_read(frd, keyframes));

        return true;
    }

    void AnimBlock::_SerializeOut(std::ofstream &frd) {
        ASSERT(false, "AnimBlock serialization is not currently implemented");
    }
} // namespace LibOpenNFS::NFS4
