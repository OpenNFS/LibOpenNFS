#include "FceFile.h"

#include "Common/Logging.h"

namespace LibOpenNFS::NFS4 {
    bool FceFile::Load(std::string const &fcePath, FceFile &fceFile) {
        LogInfo("Loading FCE File located at %s", fcePath.c_str());
        std::ifstream fce(fcePath, std::ios::in | std::ios::binary);

        fceFile.isTraffic = fcePath.find("TRAFFIC") != std::string::npos;
        bool const loadStatus{fceFile._SerializeIn(fce)};
        fce.close();

        return loadStatus;
    }

    void FceFile::Save(std::string const &fcePath, FceFile &fceFile) {
        LogInfo("Saving FCE File to %s", fcePath.c_str());
        std::ofstream fce(fcePath, std::ios::out | std::ios::binary);
        fceFile._SerializeOut(fce);
    }

    bool FceFile::_SerializeIn(std::ifstream &ifstream) {
        onfs_check(safe_read(ifstream, header));
        onfs_check(header == 0x101014);
        onfs_check(safe_read(ifstream, unknown));
        onfs_check(safe_read(ifstream, nTriangles));
        onfs_check(safe_read(ifstream, nVertices));
        onfs_check(safe_read(ifstream, nArts));
        onfs_check(safe_read(ifstream, vertTblOffset));
        onfs_check(safe_read(ifstream, normTblOffset));
        onfs_check(safe_read(ifstream, triTblOffset));
        onfs_check(safe_read(ifstream, tempStoreOffsets));
        onfs_check(safe_read(ifstream, undamagedVertsOffset));
        onfs_check(safe_read(ifstream, undamagedNormsOffset));
        onfs_check(safe_read(ifstream, damagedVertsOffset));
        onfs_check(safe_read(ifstream, damagedNormsOffset));
        onfs_check(safe_read(ifstream, unknownAreaOffset));
        onfs_check(safe_read(ifstream, driverMovementOffset));
        onfs_check(safe_read(ifstream, unknownOffsets));
        onfs_check(safe_read(ifstream, modelHalfSize));
        onfs_check(safe_read(ifstream, nDummies));
        onfs_check(safe_read(ifstream, dummyCoords));
        onfs_check(safe_read(ifstream, nParts));
        onfs_check(safe_read(ifstream, partCoords));
        onfs_check(safe_read(ifstream, partFirstVertIndices));
        onfs_check(safe_read(ifstream, partNumVertices));
        onfs_check(safe_read(ifstream, partFirstTriIndices));
        onfs_check(safe_read(ifstream, partNumTriangles));
        onfs_check(safe_read(ifstream, nColours));
        onfs_check(safe_read(ifstream, primaryColours));
        onfs_check(safe_read(ifstream, interiorColours));
        onfs_check(safe_read(ifstream, secondaryColours));
        onfs_check(safe_read(ifstream, driverHairColours));
        onfs_check(safe_read(ifstream, unknownTable));
        onfs_check(safe_read(ifstream, dummyObjectInfo));
        onfs_check(safe_read(ifstream, partNames, sizeof(char) * 64 * 64));
        onfs_check(safe_read(ifstream, unknownTable2));

        carParts.resize(nParts);

        for (uint32_t partIdx = 0; partIdx < nParts; ++partIdx) {
            carParts[partIdx].vertices.resize(partNumVertices[partIdx]);
            carParts[partIdx].normals.resize(partNumVertices[partIdx]);
            carParts[partIdx].triangles.resize(partNumTriangles[partIdx]);

            ifstream.seekg(0x2038 + vertTblOffset + (partFirstVertIndices[partIdx] * sizeof(glm::vec3)), std::ios_base::beg);
            onfs_check(safe_read(ifstream, carParts[partIdx].vertices));

            ifstream.seekg(0x2038 + normTblOffset + (partFirstVertIndices[partIdx] * sizeof(glm::vec3)), std::ios_base::beg);
            onfs_check(safe_read(ifstream, carParts[partIdx].normals));

            ifstream.seekg(0x2038 + triTblOffset + (partFirstTriIndices[partIdx] * sizeof(Triangle)), std::ios_base::beg);
            onfs_check(safe_read(ifstream, carParts[partIdx].triangles));
        }

        return true;
    }

    void FceFile::_SerializeOut(std::ofstream &ofstream) {
        ASSERT(false, "FCE output serialization is not currently implemented");
    }
} // namespace LibOpenNFS::NFS4
