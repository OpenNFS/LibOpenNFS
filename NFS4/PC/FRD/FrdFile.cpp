#include "FrdFile.h"

#include "Common/Logging.h"

namespace LibOpenNFS::NFS4 {
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

    // TODO: Hierarchically serialise the FRD, like we do for NFS3
    bool FrdFile::_SerializeIn(std::ifstream &ifstream) {
        uint32_t nPos;
        unsigned char ptrspace[44]; // some useless data from HS FRDs

        onfs_check(safe_read(ifstream, header));
        onfs_check(safe_read(ifstream, nBlocks));
        ++nBlocks;
        onfs_check(nBlocks > 1 && nBlocks <= 500);

        trackBlocks.reserve(nBlocks);
        // polygonBlocks.reserve(nBlocks);
        // extraObjectBlocks.reserve((4 * nBlocks) + 1);

        // Detect NFS3 or NFSHS
        onfs_check(safe_read(ifstream, nPos));

        if ((nPos < 0) || (nPos > 5000)) {
            version = NFSVersion::NFS_3;
        } else if (((nPos + 7) / 8) == nBlocks) {
            version = NFSVersion::NFS_4;
        } else {
            // Unknown file type
            return false;
        }

        COLFILE col{};
        memcpy(col.collID, "COLL", 4);
        col.version = 11;
        col.fileLength = 36 * nPos + 48;
        col.nBlocks = 2;
        col.xbTable[0] = 8;
        col.xbTable[1] = 24;
        // fake a texture table with only one entry to please NFS3_4/T3ED
        col.textureHead.size = 16;
        col.textureHead.xbid = XBID_TEXTUREINFO;
        col.textureHead.nrec = 1;
        col.texture = new COLTEXTUREINFO();
        // vroad XB
        col.vroadHead.size = 8 + 36 * nPos;
        col.vroadHead.xbid = XBID_VROAD;
        col.vroadHead.nrec = (uint16_t)nPos;
        col.vroad = new COLVROAD[nBlocks * 8]();
        col.hs_extra = new uint32_t[7 * nPos];

        for (uint32_t i = 0; i < nPos; i++) {
            COLVROAD vr = col.vroad[i];
            HS_VROADBLOCK vroadblk;
            onfs_check(safe_read(ifstream, vroadblk));
            vr.refPt.x = (long)(vroadblk.refPt.x * 65536);
            vr.refPt.z = (long)(vroadblk.refPt.z * 65536);
            vr.refPt.y = (long)(vroadblk.refPt.y * 65536);
            // a wild guess
            vr.unknown = (uint32_t)((vroadblk.unknown2[3] & 0xFFFF) +        // unknownLanes[2]
                                    ((vroadblk.unknown2[4] & 0xF) << 16) +   // wallKinds[0]
                                    ((vroadblk.unknown2[4] & 0xF00) << 20)); // wallKinds[1]
            if (vroadblk.normal.x >= 1.0)
                vr.normal.x = 127;
            else
                vr.normal.x = (signed char)(vroadblk.normal.x * 128);
            if (vroadblk.normal.z >= 1.0)
                vr.normal.z = 127;
            else
                vr.normal.z = (signed char)(vroadblk.normal.z * 128);
            if (vroadblk.normal.y >= 1.0)
                vr.normal.y = 127;
            else
                vr.normal.y = (signed char)(vroadblk.normal.y * 128);
            if (vroadblk.forward.x >= 1.0)
                vr.forward.x = 127;
            else
                vr.forward.x = (signed char)(vroadblk.forward.x * 128);
            if (vroadblk.forward.z >= 1.0)
                vr.forward.z = 127;
            else
                vr.forward.z = (signed char)(vroadblk.forward.z * 128);
            if (vroadblk.forward.y >= 1.0)
                vr.forward.y = 127;
            else
                vr.forward.y = (signed char)(vroadblk.forward.y * 128);
            if (vroadblk.right.x >= 1.0)
                vr.right.x = 127;
            else
                vr.right.x = (signed char)(vroadblk.right.x * 128);
            if (vroadblk.right.z >= 1.0)
                vr.right.z = 127;
            else
                vr.right.z = (signed char)(vroadblk.right.z * 128);
            if (vroadblk.right.y >= 1.0)
                vr.right.y = 127;
            else
                vr.right.y = (signed char)(vroadblk.right.y * 128);
            vr.leftWall = (long)(vroadblk.leftWall * 65536);
            vr.rightWall = (long)(vroadblk.rightWall * 65536);
            memcpy(col.hs_extra + 7 * i, &(vroadblk.unknown1[0]), 28);
        }

        std::vector<TRKBLOCK> trk(nBlocks);
        std::vector<POLYGONBLOCK> poly(nBlocks);
        std::vector<XOBJBLOCK> xobj(4 * nBlocks + 1);

        // TRKBLOCKs
        for (uint32_t block_Idx = 0; block_Idx < nBlocks; block_Idx++) {
            TRKBLOCK &b{trk.at(block_Idx)};
            POLYGONBLOCK &p{poly.at(block_Idx)};
            // 7 track polygon numbers
            onfs_check(safe_read(ifstream, p.sz));
            // 4 object polygon numbers
            for (auto &j : p.obj) {
                onfs_check(safe_read(ifstream, j.n1));
            }
            // pointer space
            ifstream.read((char *)ptrspace, 44);
            // 6 nVertices
            onfs_check(safe_read(ifstream, b.nVertices));
            onfs_check(safe_read(ifstream, b.nHiResVert));
            onfs_check(safe_read(ifstream, b.nLoResVert));
            onfs_check(safe_read(ifstream, b.nMedResVert));
            onfs_check(safe_read(ifstream, b.nVerticesDup));
            onfs_check(safe_read(ifstream, b.nObjectVert));

            // pointer space
            ifstream.read((char *)ptrspace, 8);
            // ptCentre, ptBounding == 60 bytes
            onfs_check(safe_read(ifstream, b.ptCentre));
            onfs_check(safe_read(ifstream, b.ptBounding));
            // nbdData
            onfs_check(safe_read(ifstream, b.nbdData));
            // xobj numbers
            for (uint32_t j = 4 * block_Idx; j < 4 * block_Idx + 4; j++) {
                onfs_check(safe_read(ifstream, xobj[j].nobj));
                ifstream.read((char *)ptrspace, 4);
            }
            // nVRoad is not the same as in NFS3, will change later
            onfs_check(safe_read(ifstream, b.nVRoad));
            // 2 unknown specific glm::vec3s
            onfs_check(safe_read(ifstream, b.hs_ptMin));
            onfs_check(safe_read(ifstream, b.hs_ptMax));
            ifstream.read((char *)ptrspace, 4);
            // nPositions
            onfs_check(safe_read(ifstream, b.nPositions));
            if (block_Idx == 0)
                b.nStartPos = 0;
            else
                b.nStartPos = trk[block_Idx - 1].nStartPos + trk[block_Idx - 1].nPositions;
            b.nPolygons = p.sz[4];
            // nXobj etc...
            onfs_check(safe_read(ifstream, b.nXobj));
            ifstream.read((char *)ptrspace, 4);
            onfs_check(safe_read(ifstream, b.nPolyobj));
            ifstream.read((char *)ptrspace, 4);
            onfs_check(safe_read(ifstream, b.nSoundsrc));
            ifstream.read((char *)ptrspace, 4);
            onfs_check(safe_read(ifstream, b.nLightsrc));
            ifstream.read((char *)ptrspace, 4);
            // neighbor data
            onfs_check(safe_read(ifstream, b.hs_neighbors, 32));
        }
        // TRKBLOCKDATA
        for (uint32_t block_Idx = 0; block_Idx < nBlocks; block_Idx++) {
            TRKBLOCK &b{trk.at(block_Idx)};
            POLYGONBLOCK &p{poly.at(block_Idx)};
            // vertices
            b.vert.resize(b.nVertices);
            onfs_check(safe_read(ifstream, b.vert));
            b.vertShading.resize(b.nVertices);
            onfs_check(safe_read(ifstream, b.vertShading));
            // polyData is a bit tricky
            b.polyData.resize(b.nPolygons);
            b.vroadData.resize(b.nPolygons);

            for (uint32_t j = 0; j < b.nPolygons; j++) {
                b.polyData[j].vroadEntry = j;
                b.polyData[j].flags = 0xE; // not passable
            }
            for (uint32_t j = 0; j < b.nVRoad; j++) {
                onfs_check(safe_read(ifstream, ptrspace, 10));
                int k = 0;
                onfs_check(safe_read(ifstream, k, 2));
                memcpy(b.polyData[k].hs_minmax, ptrspace, 8);
                b.polyData[k].flags = ptrspace[8];
                b.polyData[k].hs_unknown = ptrspace[9];
                if ((ptrspace[8] & 15) == 14) {
                    free(col.hs_extra);
                    free(col.texture);
                    return false;
                }
                onfs_check(safe_read(ifstream, b.vroadData.at(k)));
            }
            b.nVRoad = b.nPolygons;

            // the 4 misc. tables
            if (b.nXobj > 0) {
                b.xobj.resize(b.nXobj);
                onfs_check(safe_read(ifstream, b.xobj));
                // crossindex is f***ed up, but we don't really care
            }
            if (b.nPolyobj > 0) {
                char *buffer = (char *)malloc(b.nPolyobj * 20);
                ifstream.read((char *)buffer, 20 * b.nPolyobj);
                free(buffer);
            }
            b.nPolyobj = 0;
            if (b.nSoundsrc > 0) {
                b.soundsrc.resize(b.nSoundsrc);
                onfs_check(safe_read(ifstream, b.soundsrc));
            }
            if (b.nLightsrc > 0) {
                b.lightsrc.resize(b.nLightsrc);
                onfs_check(safe_read(ifstream, b.lightsrc));
            }

            // track polygons
            for (uint32_t j = 0; j < 7; j++)
                if (p.sz[j] != 0) {
                    p.poly[j].resize(p.sz[j]);
                    for (uint32_t k = 0; k < p.sz[j]; k++) {
                        POLYGONDATA tmppoly{};
                        onfs_check(safe_read(ifstream, tmppoly, 13));
                        for (uint32_t m = 0; m < 4; m++)
                            p.poly[j][k].vertex[m] = tmppoly.vertex[m ^ 1];
                        memcpy(&(p.poly[j][k].texture), &(tmppoly.texture), 5);
                        p.poly[j][k].unknown2 = 0xF9; // Nappe1: fixed for correct animation reading... (originally 0xF9)
                    }
                }

            // make up some fake posData
            b.posData.resize(b.nPositions);
            uint32_t k = 0;
            uint32_t i = 0;
            auto pp = p.poly[4];
            for (uint32_t j = 0; j < b.nPositions; j++) {
                b.posData[j].polygon = k;
                b.posData[j].unknown = 0;
                b.posData[j].extraNeighbor1 = -1;
                b.posData[j].extraNeighbor2 = -1;
                int l;
                do {
                    l = 0;
                    do {
                        if ((b.polyData[k].flags & 0x0f) % 14)
                            l++;
                        k++;
                        i++;
                    } while ((k < b.nPolygons) && (pp.at(i).vertex[0] == pp.at(i - 1).vertex[1]) &&
                             (pp.at(i).vertex[3] == pp.at(i - 1).vertex[2]));
                    if (((j == b.nPositions - 1) && (k < b.nPolygons)) || (k > b.nPolygons)) {
                        k = b.nPolygons;
                    }
                } while ((l == 0) && (k < b.nPolygons));
                b.posData[j].nPolygons = k - b.posData[j].polygon;
            }

            // still vroadData is missing for unpassable polygons
            for (uint32_t j = 0; j < b.nPolygons; j++) {
                if (b.polyData[j].flags == 0xE) {
                    glm::vec3 v1, v2, norm;
                    VROADDATA &v{b.vroadData.at(j)};
                    uint16_t *vno = p.poly[4][j].vertex;
                    v1.x = b.vert[vno[1]].x - b.vert[vno[3]].x;
                    v1.z = b.vert[vno[1]].z - b.vert[vno[3]].z;
                    v1.y = b.vert[vno[1]].y - b.vert[vno[3]].y;
                    v2.x = b.vert[vno[2]].x - b.vert[vno[0]].x;
                    v2.z = b.vert[vno[2]].z - b.vert[vno[0]].z;
                    v2.y = b.vert[vno[2]].y - b.vert[vno[0]].y;
                    norm.x = -v1.y * v2.z + v1.z * v2.y;
                    norm.y = -v1.z * v2.x + v1.x * v2.z;
                    norm.z = -v1.x * v2.y + v1.y * v2.x;
                    float len = (float)sqrt(norm.x * norm.x + norm.y * norm.y + norm.z * norm.z);
                    v.xNorm = (uint16_t)(norm.x * 32767 / len);
                    v.zNorm = (uint16_t)(norm.z * 32767 / len);
                    v.yNorm = (uint16_t)(norm.y * 32767 / len);
                    v1.x = (float)col.vroad[b.nStartPos].forward.x;
                    v1.z = (float)col.vroad[b.nStartPos].forward.z;
                    v1.y = (float)col.vroad[b.nStartPos].forward.y;
                    len = (float)sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
                    v.xForw = (uint16_t)(v1.x * 32767 / len);
                    v.zForw = (uint16_t)(v1.z * 32767 / len);
                    v.yForw = (uint16_t)(v1.y * 32767 / len);
                }
            }
            // POLYGONBLOCK OBJECTS
            OBJPOLYBLOCK *o = p.obj;
            auto *belong = (unsigned char *)malloc(b.nObjectVert);
            for (uint32_t j = 0; j < 4; j++, o++) {
                if (o->n1 > 0) {
                    // memset(belong, 0xFF, b.nObjectVert);
                    auto pp1 = (LPPOLYGONDATA)malloc(14 * o->n1);
                    for (k = 0; k < o->n1; k++) {
                        POLYGONDATA tmppoly;
                        onfs_check(safe_read(ifstream, tmppoly, 13));
                        for (uint32_t m = 0; m < 4; m++)
                            pp1[k].vertex[m] = tmppoly.vertex[m ^ 1];
                        memcpy(&(pp1[k].texture), &(tmppoly.texture), 5);
                        pp1[k].unknown2 = 0xFF; // will temporarily store object's #
                                                // Nappe1: Destroys AnimData??! ah... it sets it to 0xF9 later... fixing There...
                    }
                    uint32_t remn = o->n1;
                    o->nobj = 0;
                    while (remn > 0) { // there are still polygons to be connected
                        k = 0;
                        while (pp1[k].unknown2 != 0xFF)
                            k++;
                        pp1[k].unknown2 = (unsigned char)o->nobj;
                        remn--;
                        for (unsigned short l : pp1[k].vertex)
                            belong[l] = (unsigned char)o->nobj;
                        int m;
                        int l;
                        do {
                            m = 0;
                            for (k = 0; k < o->n1; k++)
                                if (pp1[k].unknown2 == 0xFF) {
                                    for (l = 0; l < 4; l++)
                                        if (belong[pp1[k].vertex[l]] == (unsigned char)o->nobj)
                                            break;
                                    if (l < 4) {
                                        remn--;
                                        m++;
                                        pp1[k].unknown2 = (unsigned char)o->nobj;
                                        for (l = 0; l < 4; l++)
                                            belong[pp1[k].vertex[l]] = (unsigned char)o->nobj;
                                    }
                                }
                        } while (m > 0); // we've been adding more polygons
                        o->nobj++;
                    }
                    o->n2 = o->nobj + xobj[4 * block_Idx + j].nobj;
                    o->types.resize(o->n2);
                    o->numpoly.resize(o->nobj);
                    o->poly = new LPPOLYGONDATA[4 * o->nobj];
                    for (uint32_t l = 0; l < o->nobj; l++) {
                        remn = 0;
                        for (k = 0; k < o->n1; k++)
                            if (pp1[k].unknown2 == l)
                                remn++;
                        o->numpoly[l] = remn;
                        o->poly[l] = (LPPOLYGONDATA)malloc(remn * sizeof(struct POLYGONDATA));
                        remn = 0;
                        for (k = 0; k < o->n1; k++)
                            if (pp1[k].unknown2 == l) {
                                memcpy(o->poly[l] + remn, pp1 + k, sizeof(struct POLYGONDATA));
                                o->poly[l][remn].unknown2 = 0xF9; // Nappe1: fixed: Loads AnimData right. Didn't work??!
                                remn++;
                            }
                    }
                    free(pp1);
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
            // XOBJs
            for (uint32_t j = 4 * block_Idx; j < 4 * block_Idx + 4; j++) {
                if (xobj[j].nobj > 0) {
                    xobj[j].obj.resize(xobj[j].nobj);
                    for (k = 0; k < xobj[j].nobj; k++) {
                        XOBJDATA &x{xobj[j].obj[k]};
                        // 3 headers == 12 bytes
                        onfs_check(safe_read(ifstream, x, 12));
                        if ((x.crosstype == 4) || (x.crosstype == 2)) // basic objects
                        {
                            onfs_check(safe_read(ifstream, x.ptRef, 12));
                        } else if (x.crosstype == 3) { // animated objects
                            // unkn3 instead of ptRef
                            onfs_check(safe_read(ifstream, x.unknown3, 12));
                        } else {
                            free(col.hs_extra);
                            free(col.texture);
                            return false; // unknown object type
                        }
                        if (p.obj[j & 3].nobj != 0) {
                            p.obj[j & 3].types[p.obj[j & 3].nobj + k] = x.crosstype;
                        }
                        // common part : vertices & polygons
                        onfs_check(safe_read(ifstream, x.AnimMemory, 4));
                        onfs_check(safe_read(ifstream, ptrspace, 4));
                        onfs_check(safe_read(ifstream, x.nVertices, 4));
                        onfs_check(safe_read(ifstream, ptrspace, 8));
                        x.vert.resize(x.nVertices);
                        x.shadingData.resize(x.nVertices);
                        onfs_check(safe_read(ifstream, x.nPolygons, 4));
                        onfs_check(safe_read(ifstream, ptrspace, 4));
                        x.polyData.resize(x.nPolygons);
                    }
                    // now the xobjdata
                    for (k = 0; k < xobj[j].nobj; k++) {
                        XOBJDATA &x{xobj[j].obj[k]};
                        if (x.crosstype == 3) { // animated-specific header
                            ifstream.read((char *)x.unknown3 + 6, 2);
                            // if (x.unknown3[6]!=4) return false;  // fails
                            // type3, objno, animLength, unknown4
                            onfs_check(safe_read(ifstream, x.type3, 6));
                            if (x.type3 != 3) {
                                free(col.hs_extra);
                                free(col.texture);
                                return false;
                            }
                            x.animData.resize(x.nAnimLength);
                            onfs_check(safe_read(ifstream, x.animData, 20 * x.nAnimLength));
                            // make a ref point from first anim position
                            x.ptRef.x = (float)(x.animData.at(0).pt.x / 65536.0);
                            x.ptRef.z = (float)(x.animData.at(0).pt.z / 65536.0);
                            x.ptRef.y = (float)(x.animData.at(0).pt.y / 65536.0);
                        }
                        // appears in REFPOLYOBJ & REFXOBJ but not in XOBJs !
                        /*				if (x->crosstype==6) { // object with byte data
                                            x->hs_type6=(char *)malloc(x->unknown2);
                                            if ((long)ar.read((char*)x->hs_type6,x->unknown2)!=x->unknown2) return false;
                                        }
                        */
                        onfs_check(safe_read(ifstream, x.vert));
                        onfs_check(safe_read(ifstream, x.shadingData));
                        for (uint32_t l = 0; l < x.nPolygons; l++) {
                            POLYGONDATA tmppoly;
                            onfs_check(safe_read(ifstream, tmppoly, 13));
                            for (uint32_t m = 0; m < 4; m++)
                                x.polyData[l].vertex[m] = tmppoly.vertex[m ^ 1];
                            memcpy(&(x.polyData[l].texture), &(tmppoly.texture), 5);
                            x.polyData[l].unknown2 = 0xF9; // Nappe1: Fixed AnimData load. Didn't work??
                                                           // what on earth for these Unknown2 definitions are used for internal checkings?
                                                           // Now it doesn't crash, but doesn't Imply Necromancers definitions... wierd...
                        }
                    }
                }
            }
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
        /*for (auto &polyBlock : polygonBlocks) {
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
        }*/

        // ofstream.write((char *) &ONFS_SIGNATURE, sizeof(uint32_t));

        ofstream.close();
    }
} // namespace LibOpenNFS::NFS4
