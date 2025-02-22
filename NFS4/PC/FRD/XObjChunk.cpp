#include "XObjChunk.h"

namespace LibOpenNFS::NFS4 {

    XObjChunk::XObjChunk(uint32_t const _nObjects, std::ifstream &frd) : nObjects(_nObjects) {
        ASSERT(this->XObjChunk::_SerializeIn(frd), "Failed to serialize XObjChunk from file stream");
    }

    bool XObjChunk::_SerializeIn(std::ifstream &frd) {
        objectHeaders.resize(nObjects);
        onfs_check(safe_read(frd, objectHeaders));

        return true;
    }

    void XObjChunk::_SerializeOut(std::ofstream &frd) {
    }
} // namespace LibOpenNFS::NFS4
