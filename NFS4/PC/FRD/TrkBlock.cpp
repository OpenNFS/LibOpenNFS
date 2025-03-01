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
        xobj2.resize(header.nPolyobj.num);
        for (size_t i = 0; i < header.nPolyobj.num; ++i) {
            auto &[unknown, type, id, pt, crossindex, unknown2] = xobj2.at(i);
            onfs_check(safe_read(frd, unknown));
            onfs_check(safe_read(frd, type));
            onfs_check(safe_read(frd, id));
            onfs_check(safe_read(frd, pt));
            onfs_check(safe_read(frd, crossindex));
            onfs_check(safe_read(frd, unknown2));
        }
        soundsrc.resize(header.nSoundsrc.num);
        onfs_check(safe_read(frd, soundsrc));
        lightsrc.resize(header.nLightsrc.num);
        onfs_check(safe_read(frd, lightsrc));
        for (uint32_t i = 0; i < 11; ++i) {
            polygonData.at(i).resize(header.sz[i]);
            onfs_check(safe_read(frd, polygonData.at(i)));
        }
        for (auto &[num, unknown] : header.nobj) {
             extraObjects.emplace_back(num, frd);
        }

        return true;
    }

    void TrkBlock::_SerializeOut(std::ofstream &frd) {
        ASSERT(false, "TrkBlock output serialization is not currently implemented");
    }
} // namespace LibOpenNFS::NFS4
