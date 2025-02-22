#include "TrkBlock.h"

namespace LibOpenNFS::NFS4 {

    TrkBlock::TrkBlock(TrkBlockHeader const &_header, std::ifstream &frd) : header(_header) {
        ASSERT(this->TrkBlock::_SerializeIn(frd), "Failed to serialize TrkBlock from file stream");
    }

    bool TrkBlock::_SerializeIn(std::ifstream &frd) {
        vertices.resize(header.nVertices);
        onfs_check(safe_read(frd, vertices));
        shadingVertices.resize(header.nVertices);
        onfs_check(safe_read(frd, shadingVertices));
        polyVroadData.resize(header.nPolygons);
        onfs_check(safe_read(frd, polyVroadData));
        xobj.resize(header.nXobj.num);
        onfs_check(safe_read(frd, xobj));
        xobj2.resize(20 * header.nPolyobj.num);
        onfs_check(safe_read(frd, xobj2));
        soundsrc.resize(header.nSoundsrc.num);
        onfs_check(safe_read(frd, soundsrc));
        soundsrc.resize(header.nLightsrc.num);
        onfs_check(safe_read(frd, lightsrc));

        for (auto const numPolys : header.sz) {
            polygonData.resize(numPolys);
            onfs_check(safe_read(frd, polygonData));
        }

        for (auto &[num, unknown] : header.nobj) {
            xobjs.emplace_back(num, frd);
        }

        return true;
    }

    void TrkBlock::_SerializeOut(std::ofstream &frd) {
    }
} // namespace LibOpenNFS::NFS4
