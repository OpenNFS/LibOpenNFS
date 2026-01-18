#include "PolyBlock.h"

using namespace LibOpenNFS::NFS3;

PolyBlock::PolyBlock(std::ifstream &frd, uint32_t const nTrackBlockPolys) : m_nTrackBlockPolys(nTrackBlockPolys) {
    ASSERT(this->PolyBlock::_SerializeIn(frd), "Failed to serialize PolyBlock from file stream");
}

bool PolyBlock::_SerializeIn(std::ifstream &ifstream) {
    for (uint32_t polyBlockIdx = 0; polyBlockIdx < NUM_POLYGON_BLOCKS; polyBlockIdx++) {
        onfs_check(safe_read(ifstream, sz[polyBlockIdx]));
        if (sz[polyBlockIdx] != 0) {
            onfs_check(safe_read(ifstream, szdup[polyBlockIdx]));
            if (szdup[polyBlockIdx] != sz[polyBlockIdx]) {
                return false;
            }
            poly[polyBlockIdx].resize(sz[polyBlockIdx]);
            onfs_check(safe_read(ifstream, poly[polyBlockIdx]));
        }
    }

    // Sanity check
    if (sz[4] != m_nTrackBlockPolys) {
        return false;
    }

    for (auto &[n1, n2, nobj, types, numpoly, poly] : obj) {
        onfs_check(safe_read(ifstream, n1));
        if (n1 > 0) {
            onfs_check(safe_read(ifstream, n2));

            types.resize(n2);
            numpoly.resize(n2);
            poly.resize(n2);

            uint32_t polygonCount = 0;
            nobj = 0;

            for (uint32_t k = 0; k < n2; ++k) {
                onfs_check(safe_read(ifstream, types[k]));

                if (types[k] == 1) {
                    onfs_check(safe_read(ifstream, numpoly[nobj]));

                    poly[nobj].resize(numpoly[nobj]);
                    onfs_check(safe_read(ifstream, poly[nobj]));

                    polygonCount += numpoly[nobj];
                    ++nobj;
                }
            }
            // n1 == total nb polygons
            if (polygonCount != n1) {
                return false;
            }
        }
    }

    return true;
}

void PolyBlock::_SerializeOut(std::ofstream &ofstream) {
    for (uint32_t polyBlockIdx = 0; polyBlockIdx < NUM_POLYGON_BLOCKS; polyBlockIdx++) {
        ofstream.write((char *)&sz[polyBlockIdx], sizeof(uint32_t));
        if (sz[polyBlockIdx] != 0) {
            ofstream.write((char *)&szdup[polyBlockIdx], sizeof(uint32_t));
            ofstream.write((char *)poly[polyBlockIdx].data(), sizeof(PolygonData) * sz[polyBlockIdx]);
        }
    }

    for (auto &[n1, n2, nobj, types, numpoly, poly] : obj) {
        ofstream.write((char *)&n1, sizeof(uint32_t));
        if (n1 > 0) {
            ofstream.write((char *)&n2, sizeof(uint32_t));
            nobj = 0;
            for (uint32_t k = 0; k < n2; ++k) {
                ofstream.write((char *)&types[k], sizeof(uint32_t));
                if (types[k] == 1) {
                    ofstream.write((char *)&numpoly[nobj], sizeof(uint32_t));
                    ofstream.write((char *)poly[nobj].data(), sizeof(PolygonData) * numpoly[nobj]);
                    ++nobj;
                }
            }
        }
    }
}
