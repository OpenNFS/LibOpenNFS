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

    bool FrdFile::_SerializeIn(std::ifstream &ifstream) {
        uint32_t nPos;
        unsigned char ptrspace[44]; // some useless data from HS FRDs

        onfs_check(safe_read(ifstream, header));
        onfs_check(safe_read(ifstream, nBlocks));
        ++nBlocks;
        onfs_check(nBlocks > 1 && nBlocks <= 500);

        trackBlocks.reserve(nBlocks);
        //polygonBlocks.reserve(nBlocks);
        //extraObjectBlocks.reserve((4 * nBlocks) + 1);

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

        // emulate the COL file
        /*memcpy(track->col.collID, "COLL", 4);
        track->col.version    = 11;
        track->col.fileLength = 36 * nPos + 48;
        track->col.nBlocks    = 2;
        track->col.xbTable[0] = 8;
        track->col.xbTable[1] = 24;
        // fake a texture table with only one entry to please NFS3_4/T3ED
        track->col.textureHead.size = 16;
        track->col.textureHead.xbid = XBID_TEXTUREINFO;
        track->col.textureHead.nrec = 1;
        track->col.texture          = new COLTEXTUREINFO();
        // vroad XB
        track->col.vroadHead.size = 8 + 36 * nPos;
        track->col.vroadHead.xbid = XBID_VROAD;
        track->col.vroadHead.nrec = (uint16_t) nPos;
        track->col.vroad          = new COLVROAD[track->nBlocks * 8]();
        track->col.hs_extra       = new uint32_t[7 * nPos];

        for (uint32_t i = 0; i < nPos; i++) {
            COLVROAD vr = track->col.vroad[i];
            HS_VROADBLOCK vroadblk;
            ifstream.read((char *) &vroadblk, 84);
            vr.refPt.x = (long) (vroadblk.refPt.x * 65536);
            vr.refPt.z = (long) (vroadblk.refPt.z * 65536);
            vr.refPt.y = (long) (vroadblk.refPt.y * 65536);
            // a wild guess
            vr.unknown = (uint32_t) ((vroadblk.unknown2[3] & 0xFFFF) +        // unknownLanes[2]
                                     ((vroadblk.unknown2[4] & 0xF) << 16) +   // wallKinds[0]
                                     ((vroadblk.unknown2[4] & 0xF00) << 20)); // wallKinds[1]
            if (vroadblk.normal.x >= 1.0)
                vr.normal.x = 127;
            else
                vr.normal.x = (signed char) (vroadblk.normal.x * 128);
            if (vroadblk.normal.z >= 1.0)
                vr.normal.z = 127;
            else
                vr.normal.z = (signed char) (vroadblk.normal.z * 128);
            if (vroadblk.normal.y >= 1.0)
                vr.normal.y = 127;
            else
                vr.normal.y = (signed char) (vroadblk.normal.y * 128);
            if (vroadblk.forward.x >= 1.0)
                vr.forward.x = 127;
            else
                vr.forward.x = (signed char) (vroadblk.forward.x * 128);
            if (vroadblk.forward.z >= 1.0)
                vr.forward.z = 127;
            else
                vr.forward.z = (signed char) (vroadblk.forward.z * 128);
            if (vroadblk.forward.y >= 1.0)
                vr.forward.y = 127;
            else
                vr.forward.y = (signed char) (vroadblk.forward.y * 128);
            if (vroadblk.right.x >= 1.0)
                vr.right.x = 127;
            else
                vr.right.x = (signed char) (vroadblk.right.x * 128);
            if (vroadblk.right.z >= 1.0)
                vr.right.z = 127;
            else
                vr.right.z = (signed char) (vroadblk.right.z * 128);
            if (vroadblk.right.y >= 1.0)
                vr.right.y = 127;
            else
                vr.right.y = (signed char) (vroadblk.right.y * 128);
            vr.leftWall  = (long) (vroadblk.leftWall * 65536);
            vr.rightWall = (long) (vroadblk.rightWall * 65536);
            memcpy(track->col.hs_extra + 7 * i, &(vroadblk.unknown1[0]), 28);
        }*/
        // Track Data
        for (uint32_t blockIdx = 0; blockIdx < nBlocks; ++blockIdx) {
            trackBlocks.push_back(TrkBlock(ifstream));
        }
        // Geometry
        /*for (uint32_t blockIdx = 0; blockIdx < nBlocks; ++blockIdx) {
            polygonBlocks.push_back(PolyBlock(ifstream, trackBlocks[blockIdx].nPolygons));
        }
        // Extra Track Geometry
        for (uint32_t blockIdx = 0; blockIdx <= 4 * nBlocks; ++blockIdx) {
            extraObjectBlocks.push_back(ExtraObjectBlock(ifstream));
        }
        // Texture Table
        onfs_check(safe_read(ifstream, nTextures));
        textureBlocks.reserve(nTextures);
        for (uint32_t tex_Idx = 0; tex_Idx < nTextures; tex_Idx++) {
            textureBlocks.push_back(TexBlock(ifstream));
        }



            // XOBJs
            for (uint32_t j = 4 * block_Idx; j < 4 * block_Idx + 4; j++) {
                if (track->xobj[j].nobj > 0) {
                    track->xobj[j].obj = new XOBJDATA[track->xobj[j].nobj]();
                    for (k = 0; k < track->xobj[j].nobj; k++) {
                        XOBJDATA *x = &(track->xobj[j].obj[k]);
                        // 3 headers == 12 bytes
                        ifstream.read((char *) x, 12);
                        if ((x->crosstype == 4) || (x->crosstype == 2)) // basic objects
                        {
                            ifstream.read((char *) &(x->ptRef), 12);
                        } else if (x->crosstype == 3) { // animated objects
                            // unkn3 instead of ptRef
                            ifstream.read((char *) x->unknown3, 12);
                        } else {
                            free(track->col.hs_extra);
                            free(track->col.texture);
                            return false; // unknown object type
                        }
                        if (p->obj[j & 3].nobj != 0) {
                            p->obj[j & 3].types[p->obj[j & 3].nobj + k] = x->crosstype;
                        }
                        // common part : vertices & polygons
                        ifstream.read((char *) &(x->AnimMemory), 4);
                        ifstream.read((char *) ptrspace, 4);
                        ifstream.read((char *) &(x->nVertices), 4);
                        ifstream.read((char *) ptrspace, 8);
                        x->vert         = new FLOATPT[12 * x->nVertices]();
                        x->unknVertices = new uint32_t[4 * x->nVertices];
                        ifstream.read((char *) &(x->nPolygons), 4);
                        ifstream.read((char *) ptrspace, 4);
                        x->polyData = new POLYGONDATA[x->nPolygons * 14]();
                    }
                    // now the xobjdata
                    for (k = 0; k < track->xobj[j].nobj; k++) {
                        XOBJDATA *x = &(track->xobj[j].obj[k]);
                        if (x->crosstype == 3) { // animated-specific header
                            ifstream.read((char *) x->unknown3 + 6, 2);
                            // if (x->unknown3[6]!=4) return false;  // fails
                            // type3, objno, animLength, unknown4
                            ifstream.read((char *) &(x->type3), 6);
                            if (x->type3 != 3) {
                                free(track->col.hs_extra);
                                free(track->col.texture);
                                return false;
                            }
                            x->animData = new ANIMDATA[20 * x->nAnimLength]();
                            ifstream.read((char *) x->animData, 20 * x->nAnimLength);
                            // make a ref point from first anim position
                            x->ptRef.x = (float) (x->animData->pt.x / 65536.0);
                            x->ptRef.z = (float) (x->animData->pt.z / 65536.0);
                            x->ptRef.y = (float) (x->animData->pt.y / 65536.0);
                        }
                        // appears in REFPOLYOBJ & REFXOBJ but not in XOBJs !
                        if (x->crosstype==6) { // object with byte data
                            x->hs_type6=(char *)malloc(x->unknown2);
                            if ((long)ifstream.read((char*)x->hs_type6,x->unknown2)!=x->unknown2) return false;
                        }

                        ifstream.read((char *) x->vert, 12 * x->nVertices);
                        ifstream.read((char *) x->unknVertices, 4 * x->nVertices);
                        for (uint32_t l = 0; l < x->nPolygons; l++) {
                            POLYGONDATA tmppoly;
                            ifstream.read((char *) &tmppoly, 13);
                            for (uint32_t m = 0;- m < 4; m++)
                                x->polyData[l].vertex[m] = tmppoly.vertex[m ^ 1];
                            memcpy(&(x->polyData[l].texture), &(tmppoly.texture), 5);
                            x->polyData[l].unknown2 = 0xF9; // Nappe1: Fixed AnimData load. Didn't work??
                                                            // what on earth for these Unknown2 definitions are used for internal checkings?
                                                            // Now it doesn't crash, but doesn't Imply Necromancers definitions... wierd...
                        }
                    }
                }
            }
        }

        uint32_t j = 4 * track->nBlocks; // Global Objects
        ifstream.read((char *) &track->xobj[j], 4);
        if (track->xobj[j].nobj > 0) {
            track->xobj[j].obj = new XOBJDATA[track->xobj[j].nobj]();
            for (uint32_t k = 0; k < track->xobj[j].nobj; k++) {
                XOBJDATA *x = &(track->nDataxobj[j].obj[k]);
                // 3 headers == 12 bytes
                ifstream.read((char *) x, 12);
                if ((x->crosstype == 4) || (x->crosstype == 2) || (x->crosstype == 1)) // basic objects + crosstype 1
                {
                    ifstream.read((char *) &(x->ptRef), 12);
                } else if (x->crosstype == 3) { // animated objects
                    // unkn3 instead of ptRef
                    ifstream.read((char *) x->unknown3, 12);
                } else
                    return false; // unknown object type
                if (p->obj[j&3].nobj!=0)
                    p->obj[j&3].types[p->obj[j&3].nobj+k]=x->crosstype;#1#
                // common part : vertices & polygons
                ifstream.read((char *) &(x->AnimMemory), 4);
                ifstream.read((char *) ptrspace, 4);
                ifstream.read((char *) &(x->nVertices), 4);
                ifstream.read((char *) ptrspace, 8);
                x->vert         = new FLOATPT[12 * x->nVertices]();
                x->unknVertices = new uint32_t[4 * x->nVertices]();
                ifstream.read((char *) &(x->nPolygons), 4);
                ifstream.read((char *) ptrspace, 4);
                x->polyData = new POLYGONDATA[x->nPolygons * 14]();
                if (x->polyData == NULL)
                    return false;
            }
            // now the xobjdata
            for (uint32_t k = 0; k < track->xobj[j].nobj; k++) {
                XOBJDATA *x = &(track->xobj[j].obj[k]);
                if (x->crosstype == 3) { // animated-specific header
                    ifstream.read((char *) x->unknown3 + 6, 2);
                    // if (x->unknown3[6]!=4) return false;  // fails
                    // type3, objno, animLength, unknown4
                    ifstream.read((char *) &(x->type3), 6);
                    if (x->type3 != 3)
                        return false;
                    x->animData = new ANIMDATA[20 * x->nAnimLength]();
                    ifstream.read((char *) x->animData, 20 * x->nAnimLength);
                    // make a ref point from first anim position
                    x->ptRef.x = (float) (x->animData->pt.x / 65536.0);
                    x->ptRef.z = (float) (x->animData->pt.z / 65536.0);
                    x->ptRef.y = (float) (x->animData->pt.y / 65536.0);
                }
                // appears in REFPOLYOBJ & REFXOBJ but not in XOBJs !
                				if (x->crosstype==6) { // object with byte data
                                    x->hs_type6=(char *)malloc(x->unknown2);
                                    if ((long)ifstream.read((char*)x->hs_type6,x->unknown2)!=x->unknown2) return false;
                                }

                ifstream.read((char *) x->vert, 12 * x->nVertices);
                ifstream.read((char *) x->unknVertices, 4 * x->nVertices);
                for (uint32_t l = 0; l < x->nPolygons; l++) {
                    POLYGONDATA tmppoly;
                    ifstream.read((char *) &tmppoly, 13);
                    for (uint32_t m = 0; m < 4; m++)
                        x->polyData[l].vertex[m] = tmppoly.vertex[m ^ 1];
                    memcpy(&(x->polyData[l].texture), &(tmppoly.texture), 5);
                    x->polyData[l].unknown2 = 0xF9; // Nappe1: Fixed AnimData load. Didn't work??
                                                    // what on earth for these Unknown2 definitions are used for internal checkings?
                                                    // Now it doesn't crash, but doesn't Imply Necromancers definitions... wierd...
                }
            }
        }

        // remainder of the FRD file
        char * hs_morexobj = (char *) malloc(100000); // no more than 100K, I hope !
        long hs_morexobjlen = ifstream.read((char*)hs_morexobj, 100000);
        if (hs_morexobjlen == 100000) return false;
        hs_morexobj = (char *) realloc(hs_morexobj, hs_morexobjlen);#1#

        // TEXTUREBLOCKs
        int m = 0;
        for (uint32_t i = 0; i < track->nBlocks; i++) {
            for (j = 0; j < 7; j++) {
                LPPOLYGONDATA pp;
                uint32_t k;
                for (k = 0, pp = track->poly[i].poly[j]; k < track->poly[i].sz[j]; k++, pp++)
                    if (pp->texture > m)
                        m = pp->texture;
            }
            for (j = 0; j < 4; j++) {
                for (uint32_t k = 0; k < track->poly[i].obj[j].nobj; k++) {
                    LPPOLYGONDATA pp;
                    uint32_t l;
                    for (l = 0, pp = track->poly[i].obj[j].poly[k]; l < track->poly[i].obj[j].numpoly[k]; l++, pp++)
                        if (pp->texture > m)
                            m = pp->texture;
                }
            }
        }
        for (uint32_t i = 0; i < 4 * track->nBlocks; i++) {
            for (j = 0; j < track->xobj[i].nobj; j++) {
                LPPOLYGONDATA pp;
                uint32_t k;
                for (k = 0, pp = track->xobj[i].obj[j].polyData; k < track->xobj[i].obj[j].nPolygons; k++, pp++)
                    if (pp->texture > m)
                        m = pp->texture;
            }
        }

        using namespace boost::filesystem;
        using namespace boost::lambda;
        std::stringstream tex_dir;
        tex_dir << TRACK_PATH << ToString(NFS_4) << "/" << track_name << "/textures/";
        path tex_path(tex_dir.str());

        uint64_t nData = std::count_if(directory_iterator(tex_path), directory_iterator(), static_cast<bool (*)(const path
    &)>(is_regular_file));

        // HOO: This is changed because some pStockBitmap can not be seen (3)
        track->nTextures = m += 15;
        // HOO: (3)
        track->texture = new TEXTUREBLOCK[m]();
        for (int32_t i = 0; i < m; i++) {
            track->texture[i].width      = 16;
            track->texture[i].height     = 16; // WHY ?????
            track->texture[i].islane     = i >> 11;
            track->texture[i].corners[2] = 1.0; // (1,0)
            track->texture[i].corners[4] = 1.0; // (1,1)
            track->texture[i].corners[5] = 1.0;
            track->texture[i].corners[7] = 1.0; // (0,1)
            track->texture[i].texture    = i;   // ANYWAY WE CAN'T FIND IT !

            if (track->texture[i].texture < nData - 1 || track->texture[i].islane) {
                track->textures[track->texture[i].texture] = LoadTexture(track->texture[i], track_name);
            }
        }*/
        // CorrectVirtualRoad();
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
} // namespace LibOpenNFS::NFS3
