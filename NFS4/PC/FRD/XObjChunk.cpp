#include "XObjChunk.h"
#include "AnimBlock.h"
#include "SpecialBlock.h"

namespace LibOpenNFS::NFS4 {
    XObjChunk::XObjChunk(uint32_t const _nObjects, std::ifstream &frd) : nObjects(_nObjects) {
        ASSERT(this->XObjChunk::_SerializeIn(frd), "Failed to serialize XObjChunk from file stream");
    }

    bool XObjChunk::_SerializeIn(std::ifstream &frd) {
        objectHeaders.resize(nObjects);
        onfs_check(safe_read(frd, objectHeaders));

        for (size_t i = 0; i < nObjects; ++i) {
            auto const &objectHeader{objectHeaders.at(i)};
            switch ((XObjHeader::Type)objectHeader.type) {
            case XObjHeader::Type::NORMAL_1:
            [[fallthrough]];
            case XObjHeader::Type::NORMAL_2:
            default:
                objectBlocks.push_back(BaseObjectBlock(objectHeader, frd));
                objectBlocks.back()._SerializeIn(frd);
                break;
            case XObjHeader::Type::ANIMATED:
                objectBlocks.push_back(AnimBlock(objectHeader, frd));
                break;
            case XObjHeader::Type::SPECIAL:
                objectBlocks.push_back(SpecialBlock(objectHeader, frd));
                break;
            }
        }

        return true;
    }

    void XObjChunk::_SerializeOut(std::ofstream &frd) {
        ASSERT(false, "XObjChunk output serialization is not currently implemented");
    }
} // namespace LibOpenNFS::NFS4
