#include "FrdFile.h"

#include "Common/Logging.h"

namespace LibOpenNFS::NFS3 {
    bool FrdFile::Load(std::string const &frdPath, FrdFile &frdFile) {
        LogInfo("Loading FRD File located at %s", frdPath.c_str());
        std::ifstream frd(frdPath, std::ios::in | std::ios::binary);

        bool const loadStatus{frdFile._SerializeIn(frd)};
        frd.close();

        return loadStatus;
    }

    void FrdFile::Save(std::string const &frdPath, FrdFile &frdFile) {
        LogInfo("Saving FRD File to %s", frdPath.c_str());
        std::ofstream frd(frdPath, std::ios::out | std::ios::binary);
        frdFile._SerializeOut(frd);
    }

    void FrdFile::MergeFRD(std::string const &frdPath, FrdFile &frdFileA, FrdFile &frdFileB) {
        // Mergearooney
        // TODO: Of course it couldn't be this simple :(
        frdFileA.nBlocks += frdFileB.nBlocks;
        frdFileA.trackBlocks.insert(frdFileA.trackBlocks.end(), frdFileB.trackBlocks.begin(),
                                    frdFileB.trackBlocks.end());
        frdFileA.polygonBlocks.insert(frdFileA.polygonBlocks.end(), frdFileB.polygonBlocks.begin(),
                                      frdFileB.polygonBlocks.end());
        frdFileA.extraObjectBlocks.insert(frdFileA.extraObjectBlocks.end(), frdFileB.extraObjectBlocks.begin(),
                                          frdFileB.extraObjectBlocks.end());

        FrdFile::Save(frdPath, frdFileA);
    }

    bool FrdFile::_SerializeIn(std::ifstream &ifstream) {
        onfs_check(safe_read(ifstream, header, HEADER_LENGTH));
        onfs_check(safe_read(ifstream, nBlocks));
        ++nBlocks;

        if (nBlocks < 1 || nBlocks > 500) {
            return false;
        }

        trackBlocks.reserve(nBlocks);
        polygonBlocks.reserve(nBlocks);
        extraObjectBlocks.reserve((4 * nBlocks) + 1);

        // Detect NFS3 or NFSHS
        int32_t hsMagic {0};
        onfs_check(safe_read(ifstream, hsMagic));

        if ((hsMagic < 0) || (hsMagic > 5000)) {
            version = NFSVersion::NFS_3;
        } else if (((hsMagic + 7) / 8) == nBlocks) {
            version = NFSVersion::NFS_4;
        } else {
            // Unknown file type
            return false;
        }

        // Back up a little, as this sizeof(int32_t) into a trackblock that we're about to deserialize
        ifstream.seekg(-4, std::ios_base::cur);

        // Track Data
        for (uint32_t blockIdx = 0; blockIdx < nBlocks; ++blockIdx) {
            trackBlocks.emplace_back(ifstream);
        }
        // Geometry
        for (uint32_t blockIdx = 0; blockIdx < nBlocks; ++blockIdx) {
            polygonBlocks.emplace_back(ifstream, trackBlocks[blockIdx].nPolygons);
        }
        // Extra Track Geometry
        for (uint32_t blockIdx = 0; blockIdx <= 4 * nBlocks; ++blockIdx) {
            extraObjectBlocks.emplace_back(ifstream);
        }
        // Texture Table
        onfs_check(safe_read(ifstream, nTextures));
        textureBlocks.reserve(nTextures);
        for (uint32_t tex_Idx = 0; tex_Idx < nTextures; tex_Idx++) {
            textureBlocks.emplace_back(ifstream);
        }

        return true;
    }

    void FrdFile::_SerializeOut(std::ofstream &ofstream) {
        // Write FRD Header
        ofstream.write((char *)&header, HEADER_LENGTH);
        uint32_t nBlocksHeader = nBlocks - 1;
        ofstream.write((char *)&nBlocksHeader, sizeof(uint32_t));

        // Track Data
        for (auto &trackBlock : trackBlocks) {
            trackBlock._SerializeOut(ofstream);
        }
        // Geometry
        for (auto &polyBlock : polygonBlocks) {
            polyBlock._SerializeOut(ofstream);
        }
        // Extra Track Geometry
        for (auto &extraObjectBlock : extraObjectBlocks) {
            extraObjectBlock._SerializeOut(ofstream);
        }
        // Texture Table
        ofstream.write((char *)&nTextures, sizeof(uint32_t));
        for (auto &textureBlock : textureBlocks) {
            textureBlock._SerializeOut(ofstream);
        }

        // ofstream.write((char *) &ONFS_SIGNATURE, sizeof(uint32_t));

        ofstream.close();
    }
} // namespace LibOpenNFS::NFS3
