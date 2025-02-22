#include "SpecialBlock.h"


namespace LibOpenNFS::NFS4 {
    SpecialBlock::SpecialBlock(XObjHeader const &_header, std::ifstream &frd) : BaseObjectBlock::BaseObjectBlock(_header, frd) {
        ASSERT(this->SpecialBlock::_SerializeIn(frd), "Failed to serialize SpecialBlock from file stream");
        ASSERT(this->BaseObjectBlock::_SerializeIn(frd), "Failed to serialize BaseObjectBlock from file stream");
    }

    bool SpecialBlock::_SerializeIn(std::ifstream &frd) {
        onfs_check(safe_read(frd, location));
        onfs_check(safe_read(frd, mass));
        onfs_check(safe_read(frd, transform));
        onfs_check(safe_read(frd, collisionDimensions));
        onfs_check(safe_read(frd, unknown3));
        onfs_check(safe_read(frd, unknown4));
        onfs_check(safe_read(frd, unknown5));

        return true;
    }

    void SpecialBlock::_SerializeOut(std::ofstream &frd) {
        ASSERT(false, "SpecialBlock output serialization is not currently implemented");
    }
} // namespace LibOpenNFS::NFS4
