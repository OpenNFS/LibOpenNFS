#include "TrkBlockHeader.h"

namespace LibOpenNFS::NFS4 {

    TrkBlockHeader::TrkBlockHeader(std::ifstream &frd) {
        ASSERT(this->TrkBlockHeader::_SerializeIn(frd), "Failed to serialize TrkBlockHeader from file stream");
    }

    bool TrkBlockHeader::_SerializeIn(std::ifstream &frd) {
        unsigned char ptrspace[44]; // some useless data from HS FRDs

        // 11 Polygon Numbers (7 track, 4 Object)
        onfs_check(safe_read(frd, sz));
        onfs_check(safe_read(frd, unknown1));
        onfs_check(safe_read(frd, nVertices));
        onfs_check(safe_read(frd, nHiResVert));
        onfs_check(safe_read(frd, nLoResVert));
        onfs_check(safe_read(frd, nMedResVert));
        onfs_check(safe_read(frd, nVerticesDup));
        onfs_check(safe_read(frd, nObjectVert));
        onfs_check(safe_read(frd, unknown2));
        onfs_check(safe_read(frd, ptCentre));
        onfs_check(safe_read(frd, ptBounding));
        onfs_check(safe_read(frd, nbdData));
        onfs_check(safe_read(frd, nobj));
        onfs_check(safe_read(frd, nPolygons));
        onfs_check(safe_read(frd, hs_ptMin));
        onfs_check(safe_read(frd, hs_ptMax));
        onfs_check(safe_read(frd, unknown3));
        onfs_check(safe_read(frd, nPositions));
        onfs_check(safe_read(frd, nXobj));
        onfs_check(safe_read(frd, nPolyobj));
        onfs_check(safe_read(frd, nSoundsrc));
        onfs_check(safe_read(frd, nLightsrc));
        onfs_check(safe_read(frd, neighbors, 32));
        return true;
    }

    void TrkBlockHeader::_SerializeOut(std::ofstream &frd) {
        ASSERT(false, "TrkBlockHeader output serialization is not currently implemented");
    }
} // namespace LibOpenNFS::NFS4
