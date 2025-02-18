#include "TrkBlock.h"

namespace LibOpenNFS::NFS4{

TrkBlock::TrkBlock(std::ifstream &frd) {
    ASSERT(this->TrkBlock::_SerializeIn(frd), "Failed to serialize TrkBlock from file stream");
}

bool TrkBlock::_SerializeIn(std::ifstream &frd) {
    // 7 track polygon numbers
    uint32_t sz[7];
    (void)safe_read(frd, sz);

    // 4 object polygon numbers
    uint32_t n1[4];
    (void)safe_read(frd, n1);

    // pointer space
    uint8_t junk[44];
    (void)safe_read(frd, junk);

    // 6 nVertices
    uint32_t nVerticesa[6];
    (void)safe_read(frd, nVerticesa);

    // pointer space
    uint8_t junk2[8];
    (void)safe_read(frd, junk2);

    // ptCentre, ptBounding == 60 bytes
    onfs_check(safe_read(frd, ptCentre, sizeof(glm::vec3)));
    onfs_check(safe_read(frd, ptBounding, sizeof(glm::vec3) * 4));
    onfs_check(safe_read(frd, nVertices));
    onfs_check(safe_read(frd, nHiResVert, sizeof(uint32_t)));
    onfs_check(safe_read(frd, nLoResVert, sizeof(uint32_t)));
    onfs_check(safe_read(frd, nMedResVert, sizeof(uint32_t)));
    onfs_check(safe_read(frd, nVerticesDup, sizeof(uint32_t)));
    onfs_check(safe_read(frd, nObjectVert, sizeof(uint32_t)));

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


    // xobj numbers
    /*for (uint32_t j = 4 * block_Idx; j < 4 * block_Idx + 4; j++) {
        ifstream.read((char *) &(track->xobj[j].nobj), 4);
        ifstream.read((char *) ptrspace, 4);
    }
    // nVRoad is not the same as in NFS3, will change later
    ifstream.read((char *) &(b->nVRoad), 4);
    // 2 unknown specific FLOATPTs
    ifstream.read((char *) &(b->hs_ptMin), 24);
    ifstream.read((char *) ptrspace, 4);
    // nPositions
    ifstream.read((char *) &(b->nPositions), 4);
    if (block_Idx == 0)
        b->nStartPos = 0;
    else
        b->nStartPos = track->trk[block_Idx - 1].nStartPos + track->trk[block_Idx - 1].nPositions;
    b->nPolygons = p->sz[4];

    // neighbor data
    ifstream.read((char *) b->hs_neighbors, 32);*/



    // Read trackblock metadata
    onfs_check(safe_read(frd, nStartPos));
    onfs_check(safe_read(frd, nPositions));
    onfs_check(safe_read(frd, nPolygons));
    onfs_check(safe_read(frd, nVRoad));
    onfs_check(safe_read(frd, nXobj, sizeof(uint32_t)));
    uint8_t junk3[4];
    onfs_check(safe_read(frd, junk3));
    onfs_check(safe_read(frd, nPolyobj, sizeof(uint32_t)));
    onfs_check(safe_read(frd, junk3));
    onfs_check(safe_read(frd, nSoundsrc, sizeof(uint32_t)));
    onfs_check(safe_read(frd, junk3));
    onfs_check(safe_read(frd, nLightsrc, sizeof(uint32_t)));
    onfs_check(safe_read(frd, junk3));

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


        // TRKBLOCKDATA
        /*for (uint32_t block_Idx = 0; block_Idx < track->nBlocks; block_Idx++) {
            TRKBLOCK *b     = &(track->trk[block_Idx]);
            POLYGONBLOCK *p = &(track->poly[block_Idx]);
            // vertices
            b->vert = new FLOATPT[b->nVertices]();
            ifstream.read((char *) b->vert, 12 * b->nVertices);
            b->unknVertices = new uint32_t[b->nVertices];
            ifstream.read((char *) b->unknVertices, 4 * b->nVertices);
            // polyData is a bit tricky
            b->polyData  = new POLYVROADDATA[b->nPolygons]();
            b->vroadData = new VROADDATA[b->nPolygons]();

            for (uint32_t j = 0; j < b->nPolygons; j++) {
                b->polyData[j].vroadEntry = j;
                b->polyData[j].flags      = 0xE; // not passable
            }
            for (uint32_t j = 0; j < b->nVRoad; j++) {
                ifstream.read((char *) ptrspace, 10);
                int k = 0;
                ifstream.read((char *) &k, 2);
                memcpy(b->polyData[k].hs_minmax, ptrspace, 8);
                b->polyData[k].flags      = ptrspace[8];
                b->polyData[k].hs_unknown = ptrspace[9];
                if ((ptrspace[8] & 15) == 14) {
                    free(track->col.hs_extra);
                    free(track->col.texture);
                    return false;
                }
                ifstream.read((char *) b->vroadData + k, 12);
            }
            b->nVRoad = b->nPolygons;

            // the 4 misc. tables
            if (b->nXobj > 0) {
                b->xobj = new REFXOBJ[b->nXobj]();
                ifstream.read((char *) b->xobj, 20 * b->nXobj);
                // crossindex is f***ed up, but we don't really care
            }
            if (b->nPolyobj > 0) {
                char *buffer = (char *) malloc(b->nPolyobj * 20);
                ifstream.read(buffer, 20 * b->nPolyobj);
                free(buffer);
            }
            b->nPolyobj = 0;
            if (b->nSoundsrc > 0) {
                b->soundsrc = new SOUNDSRC[b->nSoundsrc]();
                ifstream.read((char *) b->soundsrc, 16 * b->nSoundsrc);
            }
            if (b->nLightsrc > 0) {
                b->lightsrc = new LIGHTSRC[b->nLightsrc]();
                ifstream.read((char *) b->lightsrc, 16 * b->nLightsrc);
            }

            // track polygons
            for (uint32_t j = 0; j < 7; j++)
                if (p->sz[j] != 0) {
                    p->poly[j] = new POLYGONDATA[p->sz[j]]();
                    for (uint32_t k = 0; k < p->sz[j]; k++) {
                        POLYGONDATA tmppoly;
                        ifstream.read((char *) &tmppoly, 13);
                        for (uint32_t m = 0; m < 4; m++)
                            p->poly[j][k].vertex[m] = tmppoly.vertex[m ^ 1];
                        memcpy(&(p->poly[j][k].texture), &(tmppoly.texture), 5);
                        p->poly[j][k].unknown2 = 0xF9; // Nappe1: fixed for correct animation reading... (originally 0xF9)
                    }
                }

            // make up some fake posData
            b->posData       = new POSITIONDATA[b->nPositions]();
            uint32_t k       = 0;
            LPPOLYGONDATA pp = p->poly[4];
            for (uint32_t j = 0; j < b->nPositions; j++) {
                b->posData[j].polygon        = k;
                b->posData[j].unknown        = 0;
                b->posData[j].extraNeighbor1 = -1;
                b->posData[j].extraNeighbor2 = -1;
                int l;
                do {
                    l = 0;
                    do {
                        if ((b->polyData[k].flags & 0x0f) % 14)
                            l++;
                        k++;
                        pp++;
                    } while ((k < b->nPolygons) && (pp->vertex[0] == (pp - 1)->vertex[1]) && (pp->vertex[3] == (pp - 1)->vertex[2]));
                    if (((j == b->nPositions - 1) && (k < b->nPolygons)) || (k > b->nPolygons)) {
                        k = b->nPolygons;
                    }
                } while ((l == 0) && (k < b->nPolygons));
                b->posData[j].nPolygons = k - b->posData[j].polygon;
            }

            // still vroadData is missing for unpassable polygons
            for (uint32_t j = 0; j < b->nPolygons; j++) {
                if (b->polyData[j].flags == 0xE) {
                    FLOATPT v1, v2, norm;
                    VROADDATA *v  = b->vroadData + j;
                    uint16_t *vno = p->poly[4][j].vertex;
                    v1.x          = b->vert[vno[1]].x - b->vert[vno[3]].x;
                    v1.z          = b->vert[vno[1]].z - b->vert[vno[3]].z;
                    v1.y          = b->vert[vno[1]].y - b->vert[vno[3]].y;
                    v2.x          = b->vert[vno[2]].x - b->vert[vno[0]].x;
                    v2.z          = b->vert[vno[2]].z - b->vert[vno[0]].z;
                    v2.y          = b->vert[vno[2]].y - b->vert[vno[0]].y;
                    norm.x        = -v1.y * v2.z + v1.z * v2.y;
                    norm.y        = -v1.z * v2.x + v1.x * v2.z;
                    norm.z        = -v1.x * v2.y + v1.y * v2.x;
                    float len     = (float) sqrt(norm.x * norm.x + norm.y * norm.y + norm.z * norm.z);
                    v->xNorm      = (uint16_t) (norm.x * 32767 / len);
                    v->zNorm      = (uint16_t) (norm.z * 32767 / len);
                    v->yNorm      = (uint16_t) (norm.y * 32767 / len);
                    v1.x          = (float) track->col.vroad[b->nStartPos].forward.x;
                    v1.z          = (float) track->col.vroad[b->nStartPos].forward.z;
                    v1.y          = (float) track->col.vroad[b->nStartPos].forward.y;
                    len           = (float) sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
                    v->xForw      = (uint16_t) (v1.x * 32767 / len);
                    v->zForw      = (uint16_t) (v1.z * 32767 / len);
                    v->yForw      = (uint16_t) (v1.y * 32767 / len);
                }
            }
            // POLYGONBLOCK OBJECTS
            OBJPOLYBLOCK *o       = p->obj;
            unsigned char *belong = (unsigned char *) malloc(b->nObjectVert);
            for (uint32_t j = 0; j < 4; j++, o++) {
                if (o->n1 > 0) {
                    memset(belong, 0xFF, b->nObjectVert);
                    pp = (LPPOLYGONDATA) malloc(14 * o->n1);
                    for (k = 0; k < o->n1; k++) {
                        POLYGONDATA tmppoly;
                        ifstream.read((char *) &tmppoly, 13);
                        for (uint32_t m = 0; m < 4; m++)
                            pp[k].vertex[m] = tmppoly.vertex[m ^ 1];
                        memcpy(&(pp[k].texture), &(tmppoly.texture), 5);
                        pp[k].unknown2 = 0xFF; // will temporarily store object's #
                                               // Nappe1: Destroys AnimData??! ah... it sets it to 0xF9 later... fixing There...
                    }
                    uint32_t remn = o->n1;
                    o->nobj       = 0;
                    while (remn > 0) { // there are still polygons to be connected
                        k = 0;
                        while (pp[k].unknown2 != 0xFF)
                            k++;
                        pp[k].unknown2 = (unsigned char) o->nobj;
                        remn--;
                        for (uint32_t l = 0; l < 4; l++)
                            belong[pp[k].vertex[l]] = (unsigned char) o->nobj;
                        int m;
                        int l;
                        do {
                            m = 0;
                            for (k = 0; k < o->n1; k++)
                                if (pp[k].unknown2 == 0xFF) {
                                    for (l = 0; l < 4; l++)
                                        if (belong[pp[k].vertex[l]] == (unsigned char) o->nobj)
                                            break;
                                    if (l < 4) {
                                        remn--;
                                        m++;
                                        pp[k].unknown2 = (unsigned char) o->nobj;
                                        for (l = 0; l < 4; l++)
                                            belong[pp[k].vertex[l]] = (unsigned char) o->nobj;
                                    }
                                }
                        } while (m > 0); // we've been adding more polygons
                        o->nobj++;
                    }
                    o->n2      = o->nobj + track->xobj[4 * block_Idx + j].nobj;
                    o->types   = new uint32_t[o->n2];
                    o->numpoly = new uint32_t[o->nobj];
                    o->poly    = new LPPOLYGONDATA[4 * o->nobj];
                    for (uint32_t l = 0; l < o->nobj; l++) {
                        remn = 0;
                        for (k = 0; k < o->n1; k++)
                            if (pp[k].unknown2 == l)
                                remn++;
                        o->numpoly[l] = remn;
                        o->poly[l]    = (LPPOLYGONDATA) malloc(remn * sizeof(struct POLYGONDATA));
                        remn          = 0;
                        for (k = 0; k < o->n1; k++)
                            if (pp[k].unknown2 == l) {
                                memcpy(o->poly[l] + remn, pp + k, sizeof(struct POLYGONDATA));
                                o->poly[l][remn].unknown2 = 0xF9; // Nappe1: fixed: Loads AnimData right. Didn't work??!
                                remn++;
                            }
                    }
                    free(pp);
                    // there used to be something with REFPOLYOBJs if chunk 0
                    for (k = 0; k < o->nobj; k++) {
                        o->types[k] = 1;
                    }
                    for (k = o->nobj; k < o->n2; k++) {
                        o->types[k] = 4; // to be replaced by 3/... later
                    }
                }
            }
            free(belong);
*/
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
}
