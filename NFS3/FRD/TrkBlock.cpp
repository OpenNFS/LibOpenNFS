#include "TrkBlock.h"

using namespace LibOpenNFS::NFS3;

TrkBlock::TrkBlock(std::ifstream &frd) {
    ASSERT(this->TrkBlock::_SerializeIn(frd), "Failed to serialize TrkBlock from file stream");
}

bool TrkBlock::_SerializeIn(std::ifstream &frd) {
    onfs_check(safe_read(frd, ptCentre));
    onfs_check(safe_read(frd, ptBounding));
    onfs_check(safe_read(frd, nVertices));
    onfs_check(safe_read(frd, nHiResVert));
    onfs_check(safe_read(frd, nLoResVert));
    onfs_check(safe_read(frd, nMedResVert));
    onfs_check(safe_read(frd, nVerticesDup));
    onfs_check(safe_read(frd, nObjectVert));

    if (nVertices == 0) {
        return false;
    }

    // Read Vertices
    vert.resize(nVertices);
    onfs_check(safe_read(frd, vert));

    // Read Vertices
    vertShading.resize(nVertices);
    onfs_check(safe_read(frd, vertShading));

    // Read neighbouring block data
    onfs_check(safe_read(frd, nbdData, 4 * 0x12c));

    // Read trackblock metadata
    onfs_check(safe_read(frd, nStartPos));
    onfs_check(safe_read(frd, nPositions));
    onfs_check(safe_read(frd, nPolygons));
    onfs_check(safe_read(frd, nVRoad));
    onfs_check(safe_read(frd, nXobj));
    onfs_check(safe_read(frd, nPolyobj));
    onfs_check(safe_read(frd, nSoundsrc));
    onfs_check(safe_read(frd, nLightsrc));

    // Read track position data
    posData.resize(nPositions);
    onfs_check(safe_read(frd, posData));

    // Read virtual road polygons
    polyData.resize(nPolygons);
    onfs_check(safe_read(frd, polyData));

    // Read virtual road spline data
    vroadData.resize(nVRoad);
    onfs_check(safe_read(frd, vroadData));

    // Read Extra object references
    xobj.resize(nXobj);
    onfs_check(safe_read(frd, xobj));

    // ?? Read unknown
    polyObj.resize(nPolyobj);
    onfs_check(safe_read(frd, polyObj));
    // nPolyobj = 0;

    // Get the sound and light sources
    soundsrc.resize(nSoundsrc);
    onfs_check(safe_read(frd, soundsrc));

    lightsrc.resize(nLightsrc);
    onfs_check(safe_read(frd, lightsrc));

    return true;
}

void TrkBlock::_SerializeOut(std::ofstream &frd) {
    frd.write((char *)&ptCentre, sizeof(glm::vec3));
    frd.write((char *)&ptBounding, sizeof(glm::vec3) * 4);
    frd.write((char *)&nVertices, sizeof(uint32_t));
    frd.write((char *)&nHiResVert, sizeof(uint32_t));
    frd.write((char *)&nLoResVert, sizeof(uint32_t));
    frd.write((char *)&nMedResVert, sizeof(uint32_t));
    frd.write((char *)&nVerticesDup, sizeof(uint32_t));
    frd.write((char *)&nObjectVert, sizeof(uint32_t));
    frd.write((char *)vert.data(), sizeof(glm::vec3) * nVertices);
    frd.write((char *)vertShading.data(), sizeof(uint32_t) * nVertices);
    frd.write((char *)nbdData, 4 * 0x12c);
    frd.write((char *)&nStartPos, sizeof(uint32_t));
    frd.write((char *)&nPositions, sizeof(uint32_t));
    frd.write((char *)&nPolygons, sizeof(uint32_t));
    frd.write((char *)&nVRoad, sizeof(uint32_t));
    frd.write((char *)&nXobj, sizeof(uint32_t));
    frd.write((char *)&nPolyobj, sizeof(uint32_t));
    frd.write((char *)&nSoundsrc, sizeof(uint32_t));
    frd.write((char *)&nLightsrc, sizeof(uint32_t));
    frd.write((char *)posData.data(), sizeof(PositionData) * nPositions);
    frd.write((char *)polyData.data(), sizeof(PolyVRoadData) * nPolygons);
    frd.write((char *)vroadData.data(), sizeof(VRoadData) * nVRoad);
    frd.write((char *)xobj.data(), sizeof(RefExtraObject) * nXobj);
    frd.write((char *)polyObj.data(), sizeof(PolyObject) * nPolyobj);
    frd.write((char *)soundsrc.data(), sizeof(SoundSource) * nSoundsrc);
    frd.write((char *)lightsrc.data(), sizeof(LightSource) * nLightsrc);
}