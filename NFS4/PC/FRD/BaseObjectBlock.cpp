#include "BaseObjectBlock.h"

namespace LibOpenNFS::NFS4 {

    BaseObjectBlock::BaseObjectBlock(XObjHeader const &_header, std::ifstream &frd) : header(_header) {

    }

    bool BaseObjectBlock::_SerializeIn(std::ifstream &frd) {
        vertices.resize(header.nVertices);
        shadingVertices.resize(header.nVertices);
        polygons.resize(header.nPolygons);
        onfs_check(safe_read(frd, vertices));
        onfs_check(safe_read(frd, shadingVertices));
        onfs_check(safe_read(frd, polygons));

        return true;
    }

    void BaseObjectBlock::_SerializeOut(std::ofstream &frd) {
        ASSERT(false, "BaseObjectBlock output serialization is not currently implemented");
    }
} // namespace LibOpenNFS::NFS4
