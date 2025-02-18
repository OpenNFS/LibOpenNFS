#include "NFS4Loader.h"

#include "FCE/FceFile.h"
#include <Common/Logging.h>
#include <Common/Utils.h>
#include <Shared/VivFile.h>

#include <sstream>

namespace LibOpenNFS::NFS4 {
    Car Loader::LoadCar(std::string const &carBasePath, std::string const &carOutPath, NFSVersion version) {
        LogInfo("Loading NFS4 car from %s into %s", carBasePath.c_str(), carOutPath.c_str());

        std::filesystem::path p(carBasePath);
        std::string carName = p.filename().replace_extension("").string();

        std::stringstream vivPath, fcePath, fshPath, fedataPath;
        vivPath << carBasePath;
        fcePath << carOutPath;
        fedataPath << carOutPath << "/fedata.eng";

        if (version == NFSVersion::NFS_4) {
            vivPath << "/car.viv";
            fcePath << "/car.fce";
        } else {
            // MCO
            vivPath << ".viv";
            fcePath << "/part.fce";
            fshPath << "../resources/MCO/Data/skins/" << carName.substr(0, carName.size() - 1) << "dec.fsh";
        }

        Shared::VivFile vivFile;
        FceFile fceFile;
        FedataFile fedataFile;

        if (std::filesystem::exists(carOutPath)) {
            LogInfo("VIV has already been extracted to %s, skipping", carOutPath.c_str());
        } else {
            ASSERT(Shared::VivFile::Load(vivPath.str(), vivFile), "Could not open VIV file: " << vivPath.str());
            ASSERT(Shared::VivFile::Extract(carOutPath, vivFile), "Could not extract VIV file: " << vivPath.str() << "to: " << carOutPath);
        }
        ASSERT(NFS4::FceFile::Load(fcePath.str(), fceFile), "Could not load FCE file: " << fcePath.str());
        if (!FedataFile::Load(fedataPath.str(), fedataFile, fceFile.nColours)) {
            LogWarning("Could not load FeData file: %s", fedataPath.str().c_str());
        }

        if (version == NFSVersion::MCO) {
            if (std::filesystem::exists(fshPath.str())) {
                TextureUtils::ExtractQFS(fshPath.str(), fshPath.str() + "/Textures/");
            } else {
                LogInfo("Can't find MCO car texture at %s (More work needed to identify when certain fsh's are used)",
                        fshPath.str().c_str());
            }
        }

        Car::MetaData const carData{_ParseAssetData(fceFile, fedataFile, version)};

        return Car(carData, version, carName);
    }

    Track Loader::LoadTrack(std::string const &trackBasePath, std::string const &trackOutPath) {
        LogInfo("Loading Track located at %s", trackBasePath.c_str());
        std::filesystem::path p(trackBasePath);
        std::string trackName = p.filename().string();
        std::string frdPath, canPath;
        std::string strip = "k0", trackNameStripped = trackName;
        size_t pos = trackName.find(strip);
        if (pos != std::string::npos) {
            trackNameStripped.replace(pos, strip.size(), "");
        }
        Track track(NFSVersion::NFS_4, trackNameStripped, trackBasePath, trackName);

        frdPath = trackBasePath + "/" + "tr.frd";
        canPath = trackBasePath + "/" + trackNameStripped + "/tr00.can";

        FrdFile frdFile;
        Shared::CanFile canFile;

        // Load FRD file to get track block specific data
        ASSERT(FrdFile::Load(frdPath, frdFile), "Could not load FRD file: " << frdPath);
        // Load camera intro/outro animation data
        ASSERT(Shared::CanFile::Load(canPath, canFile), "Could not load CAN file (camera animation): " << canPath);

        track.nBlocks = frdFile.nBlocks;
        track.cameraAnimation = canFile.animPoints;
        // track.trackTextureAssets = _ParseTextures(frdFile, track, trackOutPath);
        // track.trackBlocks = _ParseTRKModels(frdFile, track);
        // track.globalObjects = _ParseCOLModels(colFile, track, frdFile.textureBlocks);
        // track.virtualRoad = _ParseVirtualRoad(colFile);

        LogInfo("Track loaded successfully");

        return track;
    }

    Car::MetaData Loader::_ParseAssetData(FceFile const &fceFile, FedataFile const &fedataFile, NFSVersion version) {
        LogInfo("Parsing FCE File into ONFS Structures");
        // All Vertices are stored so that the model is rotated 90 degs on X, 180 on Z. Remove this at Vert load time.
        Car::MetaData carMetadata;

        // Go get car metadata from FEDATA
        carMetadata.name = fedataFile.menuName;

        // Grab colours
        for (uint8_t colourIdx = 0; colourIdx < fceFile.nColours; ++colourIdx) {
            if (fceFile.nColours == 1) {
                break;
            }
            auto [Hp, Sp, Bp, Tp] = fceFile.primaryColours[colourIdx];
            auto [Hs, Ss, Bs, Ts] = fceFile.secondaryColours[colourIdx];
            Car::Colour originalPrimaryColour(fedataFile.primaryColourNames[colourIdx], TextureUtils::HSLToRGB(glm::vec4(Hp, Sp, Bp, Tp)),
                                              TextureUtils::HSLToRGB(glm::vec4(Hs, Ss, Bs, Ts)));
            carMetadata.colours.emplace_back(originalPrimaryColour);
        }

        for (uint32_t dummyIdx = 0; dummyIdx < fceFile.nDummies; ++dummyIdx) {
            Car::Dummy dummy(fceFile.dummyObjectInfo[dummyIdx].data, fceFile.dummyCoords[dummyIdx] * NFS4_SCALE_FACTOR);
            carMetadata.dummies.emplace_back(dummy);
        }

        for (uint32_t partIdx = 0; partIdx < fceFile.nParts; ++partIdx) {
            std::vector<uint32_t> indices;
            std::vector<uint32_t> polygonFlags;
            std::vector<glm::vec3> vertices;
            std::vector<glm::vec3> normals;
            std::vector<glm::vec2> uvs;

            std::string part_name(fceFile.partNames[partIdx]);
            glm::vec3 center{fceFile.partCoords[partIdx] * NFS4_SCALE_FACTOR};
            FceFile::CarPart const &part{fceFile.carParts[partIdx]};

            vertices.reserve(fceFile.partNumVertices[partIdx]);
            normals.reserve(fceFile.partNumVertices[partIdx]);
            polygonFlags.reserve(fceFile.partNumTriangles[partIdx] * 3);
            indices.reserve(fceFile.partNumTriangles[partIdx] * 3);
            uvs.reserve(fceFile.partNumTriangles[partIdx] * 3);

            for (uint32_t vert_Idx = 0; vert_Idx < fceFile.partNumVertices[partIdx]; ++vert_Idx) {
                vertices.emplace_back(part.vertices[vert_Idx] * NFS4_SCALE_FACTOR);
            }
            for (uint32_t normal_Idx = 0; normal_Idx < fceFile.partNumVertices[partIdx]; ++normal_Idx) {
                normals.emplace_back(part.normals[normal_Idx] * NFS4_SCALE_FACTOR);
            }
            for (uint32_t tri_Idx = 0; tri_Idx < fceFile.partNumTriangles[partIdx]; ++tri_Idx) {
                polygonFlags.emplace_back(part.triangles[tri_Idx].polygonFlags);
                polygonFlags.emplace_back(part.triangles[tri_Idx].polygonFlags);
                polygonFlags.emplace_back(part.triangles[tri_Idx].polygonFlags);
                indices.emplace_back(part.triangles[tri_Idx].vertex[0]);
                indices.emplace_back(part.triangles[tri_Idx].vertex[1]);
                indices.emplace_back(part.triangles[tri_Idx].vertex[2]);
                if (fceFile.isTraffic) {
                    uvs.emplace_back(part.triangles[tri_Idx].uvTable[0], part.triangles[tri_Idx].uvTable[3]);
                    uvs.emplace_back(part.triangles[tri_Idx].uvTable[1], part.triangles[tri_Idx].uvTable[4]);
                    uvs.emplace_back(part.triangles[tri_Idx].uvTable[2], part.triangles[tri_Idx].uvTable[5]);
                } else {
                    uvs.emplace_back(part.triangles[tri_Idx].uvTable[0], version == NFSVersion::MCO
                                                                             ? 1.0f - part.triangles[tri_Idx].uvTable[3]
                                                                             : part.triangles[tri_Idx].uvTable[3]);
                    uvs.emplace_back(part.triangles[tri_Idx].uvTable[1], version == NFSVersion::MCO
                                                                             ? 1.0f - part.triangles[tri_Idx].uvTable[4]
                                                                             : part.triangles[tri_Idx].uvTable[4]);
                    uvs.emplace_back(part.triangles[tri_Idx].uvTable[2], version == NFSVersion::MCO
                                                                             ? 1.0f - part.triangles[tri_Idx].uvTable[5]
                                                                             : part.triangles[tri_Idx].uvTable[5]);
                }
            }
            carMetadata.meshes.emplace_back(part_name, vertices, uvs, normals, indices, polygonFlags, center);
        }

        return carMetadata;
    }

    /* std::vector<TrackBlock> NFS4::ParseTRKModels(const std::shared_ptr<TRACK> &track) {
         std::vector<TrackBlock> track_blocks = std::vector<TrackBlock>();
         glm::quat rotationMatrix =
           glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))); // All Vertices are stored so that the model is rotated 90 degs on X.
     Remove this at Vert load time.

         /* TRKBLOCKS - BASE TRACK GEOMETRY #1#
         for (uint32_t i = 0; i < track->nBlocks; i++) {
             // Get Verts from Trk block, indices from associated polygon block
             TRKBLOCK trk_block         = track->trk[i];
             POLYGONBLOCK polygon_block = track->poly[i];
             TrackBlock current_track_block(i, rotationMatrix * glm::vec3(trk_block.ptCentre.x / 10, trk_block.ptCentre.y / 10,
     trk_block.ptCentre.z / 10)); glm::vec3 trk_block_center = rotationMatrix * glm::vec3(0, 0, 0);

             // Light sources
             for (uint32_t j = 0; j < trk_block.nLightsrc; j++) {
                 glm::vec3 light_center =
                   rotationMatrix *
                   glm::vec3((trk_block.lightsrc[j].refpoint.x / 65536.0) / 10, (trk_block.lightsrc[j].refpoint.y / 65536.0) / 10,
     (trk_block.lightsrc[j].refpoint.z / 65536.0) / 10); current_track_block.lights.emplace_back(Entity(i, j, NFS_4, LIGHT,
     MakeLight(light_center, trk_block.lightsrc[j].type), 0));
             }

             for (uint32_t s = 0; s < trk_block.nSoundsrc; s++) {
                 glm::vec3 sound_center =
                   rotationMatrix *
                   glm::vec3((trk_block.soundsrc[s].refpoint.x / 65536.0) / 10, (trk_block.soundsrc[s].refpoint.y / 65536.0) / 10,
     (trk_block.soundsrc[s].refpoint.z / 65536.0) / 10); current_track_block.sounds.emplace_back(Entity(i, s, NFS_4, SOUND,
     Sound(sound_center, trk_block.soundsrc[s].type), 0));
             }

             // Get Object vertices
             std::vector<glm::vec3> obj_verts;
             std::vector<glm::vec4> obj_shading_verts;
             for (uint32_t v = 0; v < trk_block.nObjectVert; v++) {
                 obj_verts.emplace_back(rotationMatrix * glm::vec3(trk_block.vert[v].x / 10, trk_block.vert[v].y / 10, trk_block.vert[v].z /
     10)); uint32_t shading_data = trk_block.unknVertices[v]; obj_shading_verts.emplace_back( glm::vec4(((shading_data >> 16) & 0xFF) /
     255.0f, ((shading_data >> 8) & 0xFF) / 255.0f, (shading_data & 0xFF) / 255.0f, ((shading_data >> 24) & 0xFF) / 255.0f));
             }
             // 4 OBJ Poly blocks
             for (uint32_t j = 0; j < 4; j++) {
                 OBJPOLYBLOCK obj_polygon_block = polygon_block.obj[j];
                 if (obj_polygon_block.n1 > 0) {
                     // Iterate through objects in objpoly block up to num objects
                     for (uint32_t k = 0; k < obj_polygon_block.nobj; k++) {
                         // TODO: Animated objects here, obj_polygon_block.types
                         // Mesh Data
                         std::vector<uint32_t> vertex_indices;
                         std::vector<glm::vec2> uvs;
                         std::vector<uint32_t> texture_indices;
                         std::vector<glm::vec3> norms;
                         FLOATPT norm_floatpt = {0.f, 0.f, 0.f};
                         // Get Polygons in object
                         LPPOLYGONDATA object_polys = obj_polygon_block.poly[k];
                         for (uint32_t p = 0; p < obj_polygon_block.numpoly[k]; p++) {
                             TEXTUREBLOCK texture_for_block = track->texture[object_polys[p].texture];
                             Texture gl_texture             = track->textures[texture_for_block.texture];

                             glm::vec3 normal = rotationMatrix * CalculateQuadNormal(PointToVec(trk_block.vert[object_polys[p].vertex[0]]),
                                                                                     PointToVec(trk_block.vert[object_polys[p].vertex[1]]),
                                                                                     PointToVec(trk_block.vert[object_polys[p].vertex[2]]),
                                                                                     PointToVec(trk_block.vert[object_polys[p].vertex[3]]));
                             norms.emplace_back(normal);
                             norms.emplace_back(normal);
                             norms.emplace_back(normal);
                             norms.emplace_back(normal);
                             norms.emplace_back(normal);
                             norms.emplace_back(normal);

                             vertex_indices.emplace_back(object_polys[p].vertex[0]);
                             vertex_indices.emplace_back(object_polys[p].vertex[1]);
                             vertex_indices.emplace_back(object_polys[p].vertex[2]);
                             vertex_indices.emplace_back(object_polys[p].vertex[0]);
                             vertex_indices.emplace_back(object_polys[p].vertex[2]);
                             vertex_indices.emplace_back(object_polys[p].vertex[3]);

                             std::vector<glm::vec2> transformedUVs; // = GenerateUVs(NFS_4, OBJ_POLY, object_polys[p].hs_texflags,
     gl_texture, texture_for_block); uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                             texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                             texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                             texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                             texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                             texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                             texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                         }
                         current_track_block.objects.emplace_back(
                           Entity(i, (j + 1) * (k + 1), NFS_4, OBJ_POLY, TrackModel(obj_verts, norms, uvs, texture_indices, vertex_indices,
     obj_shading_verts, trk_block_center), 0));
                     }
                 }
             }

             /* XOBJS - EXTRA OBJECTS #1#
             for (uint32_t l = (i * 4); l < (i * 4) + 4; l++) {
                 for (uint32_t j = 0; j < track->xobj[l].nobj; j++) {
                     XOBJDATA *x = &(track->xobj[l].obj[j]);
                     // common part : vertices & polygons
                     std::vector<glm::vec3> verts;
                     std::vector<glm::vec4> xobj_shading_verts;
                     for (uint32_t k = 0; k < x->nVertices; k++, x->vert++) {
                         verts.emplace_back(rotationMatrix * glm::vec3(x->ptRef.x / 10 + x->vert->x / 10, x->ptRef.y / 10 + x->vert->y / 10,
     x->ptRef.z / 10 + x->vert->z / 10)); uint32_t shading_data = x->unknVertices[k];
                         // RGBA
                         xobj_shading_verts.emplace_back(glm::vec4(
                           ((shading_data >> 16) & 0xFF) / 255.0f, ((shading_data >> 8) & 0xFF) / 255.0f, (shading_data & 0xFF) / 255.0f,
     ((shading_data >> 24) & 0xFF) / 255.0f));
                     }
                     std::vector<uint32_t> vertex_indices;
                     std::vector<glm::vec2> uvs;
                     std::vector<uint32_t> texture_indices;
                     std::vector<glm::vec3> norms;

                     for (uint32_t k = 0; k < x->nPolygons; k++, x->polyData++) {
                         TEXTUREBLOCK texture_for_block = track->texture[x->polyData->texture];
                         Texture gl_texture             = track->textures[texture_for_block.texture];

                         glm::vec3 normal = rotationMatrix * CalculateQuadNormal(PointToVec(verts[x->polyData->vertex[0]]),
                                                                                 PointToVec(verts[x->polyData->vertex[1]]),
                                                                                 PointToVec(verts[x->polyData->vertex[2]]),
                                                                                 PointToVec(verts[x->polyData->vertex[3]]));
                         norms.emplace_back(normal);
                         norms.emplace_back(normal);
                         norms.emplace_back(normal);
                         norms.emplace_back(normal);
                         norms.emplace_back(normal);
                         norms.emplace_back(normal);

                         vertex_indices.emplace_back(x->polyData->vertex[0]);
                         vertex_indices.emplace_back(x->polyData->vertex[1]);
                         vertex_indices.emplace_back(x->polyData->vertex[2]);
                         vertex_indices.emplace_back(x->polyData->vertex[0]);
                         vertex_indices.emplace_back(x->polyData->vertex[2]);
                         vertex_indices.emplace_back(x->polyData->vertex[3]);

                         std::vector<glm::vec2> transformedUVs; // = GenerateUVs(NFS_4, XOBJ, x->polyData->hs_texflags,gl_texture,
     texture_for_block); uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                         texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                         texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                         texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                         texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                         texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                         texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                     }
                     current_track_block.objects.emplace_back(
                       Entity(i, l, NFS_4, XOBJ, TrackModel(verts, norms, uvs, texture_indices, vertex_indices, xobj_shading_verts,
     trk_block_center), 0));
                 }
             }

             // Mesh Data
             std::vector<uint32_t> vertex_indices;
             std::vector<glm::vec2> uvs;
             std::vector<uint32_t> texture_indices;
             std::vector<glm::vec3> verts;
             std::vector<glm::vec4> trk_block_shading_verts;
             std::vector<glm::vec3> norms;
             for (int32_t j = 0; j < trk_block.nVertices; j++) {
                 verts.emplace_back(rotationMatrix * glm::vec3(trk_block.vert[j].x / 10, trk_block.vert[j].y / 10, trk_block.vert[j].z /
     10));
                 // Break uint32_t of RGB into 4 normalised floats and store into vec4
                 uint32_t shading_data = trk_block.unknVertices[j];
                 trk_block_shading_verts.emplace_back(
                   glm::vec4(((shading_data >> 16) & 0xFF) / 255.0f, ((shading_data >> 8) & 0xFF) / 255.0f, (shading_data & 0xFF) / 255.0f,
     ((shading_data >> 24) & 0xFF) / 255.0f));
             }

             // Get indices from Chunk 4 and 5 for High Res polys, Chunk 6 for Road Lanes
             for (int32_t chnk = 4; chnk <= 6; chnk++) {
                 if ((chnk == 6) && (trk_block.nVertices <= trk_block.nHiResVert))
                     continue;
                 LPPOLYGONDATA poly_chunk = polygon_block.poly[chnk];
                 for (uint32_t k = 0; k < polygon_block.sz[chnk]; k++) {
                     TEXTUREBLOCK texture_for_block = track->texture[poly_chunk[k].texture];
                     Texture gl_texture             = track->textures[texture_for_block.texture];

                     glm::vec3 normal = rotationMatrix * CalculateQuadNormal(PointToVec(trk_block.vert[poly_chunk[k].vertex[0]]),
                                                                             PointToVec(trk_block.vert[poly_chunk[k].vertex[1]]),
                                                                             PointToVec(trk_block.vert[poly_chunk[k].vertex[2]]),
                                                                             PointToVec(trk_block.vert[poly_chunk[k].vertex[3]]));
                     norms.emplace_back(normal);
                     norms.emplace_back(normal);
                     norms.emplace_back(normal);
                     norms.emplace_back(normal);
                     norms.emplace_back(normal);
                     norms.emplace_back(normal);

                     vertex_indices.emplace_back(poly_chunk[k].vertex[0]);
                     vertex_indices.emplace_back(poly_chunk[k].vertex[1]);
                     vertex_indices.emplace_back(poly_chunk[k].vertex[2]);
                     vertex_indices.emplace_back(poly_chunk[k].vertex[0]);
                     vertex_indices.emplace_back(poly_chunk[k].vertex[2]);
                     vertex_indices.emplace_back(poly_chunk[k].vertex[3]);

                     std::vector<glm::vec2> transformedUVs; // = GenerateUVs(NFS_4, chnk == 6 ? LANE : ROAD,poly_chunk[k].hs_texflags,
     gl_texture,texture_for_block); uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                     texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                     texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                     texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                     texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                     texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                     texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                 }

                 if (chnk == 6) {
                     current_track_block.lanes.emplace_back(
                       Entity(i, -1, NFS_4, LANE, TrackModel(verts, norms, uvs, texture_indices, vertex_indices, trk_block_shading_verts,
     trk_block_center), 0)); } else { current_track_block.track.emplace_back( Entity(i, -1, NFS_4, ROAD, TrackModel(verts, norms, uvs,
     texture_indices, vertex_indices, trk_block_shading_verts, trk_block_center), 0));
                 }
             }
             track_blocks.emplace_back(current_track_block);
         }

         // Animated, Global objects
         uint32_t globalObjIdx = 4 * track->nBlocks; // Global Objects
         for (uint32_t j = 0; j < track->xobj[globalObjIdx].nobj; j++) {
             XOBJDATA *x = &(track->xobj[globalObjIdx].obj[j]);
             // common part : vertices & polygons
             std::vector<glm::vec3> verts;
             std::vector<glm::vec4> xobj_shading_verts;
             for (uint32_t k = 0; k < x->nVertices; k++, x->vert++) {
                 verts.emplace_back(rotationMatrix * glm::vec3(x->vert->x / 10, x->vert->y / 10, x->vert->z / 10));
                 uint32_t shading_data = x->unknVertices[k];
                 // RGBA
                 xobj_shading_verts.emplace_back(
                   glm::vec4(((shading_data >> 16) & 0xFF) / 255.0f, ((shading_data >> 8) & 0xFF) / 255.0f, (shading_data & 0xFF) / 255.0f,
     ((shading_data >> 24) & 0xFF) / 255.0f));
             }
             std::vector<uint32_t> vertex_indices;
             std::vector<glm::vec2> uvs;
             std::vector<uint32_t> texture_indices;
             std::vector<glm::vec3> norms;

             for (uint32_t k = 0; k < x->nPolygons; k++, x->polyData++) {
                 TEXTUREBLOCK texture_for_block = track->texture[x->polyData->texture];
                 Texture gl_texture             = track->textures[texture_for_block.texture];

                 glm::vec3 normal = rotationMatrix * CalculateQuadNormal(PointToVec(verts[x->polyData->vertex[0]]),
                                                                         PointToVec(verts[x->polyData->vertex[1]]),
                                                                         PointToVec(verts[x->polyData->vertex[2]]),
                                                                         PointToVec(verts[x->polyData->vertex[3]]));
                 norms.emplace_back(normal);
                 norms.emplace_back(normal);
                 norms.emplace_back(normal);
                 norms.emplace_back(normal);
                 norms.emplace_back(normal);
                 norms.emplace_back(normal);

                 vertex_indices.emplace_back(x->polyData->vertex[0]); // FL
                 vertex_indices.emplace_back(x->polyData->vertex[1]); // FR
                 vertex_indices.emplace_back(x->polyData->vertex[2]); // BR
                 vertex_indices.emplace_back(x->polyData->vertex[0]); // FL
                 vertex_indices.emplace_back(x->polyData->vertex[2]); // BR
                 vertex_indices.emplace_back(x->polyData->vertex[3]); // BL

                 std::vector<glm::vec2> transformedUVs; // = GenerateUVs(NFS_4, XOBJ, x->polyData->hs_texflags,
     gl_texture,texture_for_block); uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                 texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                 texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                 texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                 texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                 texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
                 texture_indices.emplace_back(hsStockTextureIndexRemap(texture_for_block.texture));
             }
             glm::vec3 position =
               rotationMatrix * glm::vec3(static_cast<float>(x->ptRef.x / 65536.0) / 10, static_cast<float>(x->ptRef.y / 65536.0) / 10,
     static_cast<float>(x->ptRef.z / 65536.0) / 10); track->global_objects.emplace_back(Entity(-1, j, NFS_4, GLOBAL, TrackModel(verts,
     norms, uvs, texture_indices, vertex_indices, xobj_shading_verts, position), 0));
         }

         return track_blocks;
     }

     Texture NFS4::LoadTexture(TEXTUREBLOCK track_texture, const std::string &track_name) {
         std::stringstream filename;
         std::stringstream filename_alpha;

         if (track_texture.islane) {
             filename << "../resources/sfx/" << std::setfill('0') << std::setw(4) << (track_texture.texture - 2048) + 9 << ".BMP";
             filename_alpha << "../resources/sfx/" << std::setfill('0') << std::setw(4) << (track_texture.texture - 2048) + 9 << "-a.BMP";
         } else {
             filename << TRACK_PATH << ToString(NFS_4) << "/" << track_name << "/textures/" << std::setfill('0') << std::setw(4) <<
     track_texture.texture << ".BMP"; filename_alpha << TRACK_PATH << ToString(NFS_4) << "/" << track_name << "/textures/" <<
     std::setfill('0') << std::setw(4) << track_texture.texture << "-a.BMP";
         }

         // Width and height data isn't set properly in FRD loader so deduce from bmp
         GLubyte *data;
         GLsizei width;
         GLsizei height;

         if (!ImageLoader::LoadBmpCustomAlpha(filename.str().c_str(), &data, &width, &height, 0)) {
             std::cerr << "Texture " << filename.str() << " or " << filename_alpha.str() << " did not load succesfully!" << std::endl;
             // If the texture is missing, load a "MISSING" texture of identical size.
             ASSERT(ImageLoader::LoadBmpWithAlpha("../resources/misc/missing.bmp", "../resources/misc/missing-a.bmp", &data, &width,
     &height), "Even the 'missing' texture is missing!"); return Texture((unsigned int) track_texture.texture, data, static_cast<unsigned
     int>(track_texture.width), static_cast<unsigned int>(track_texture.height));
         }

         return Texture((unsigned int) track_texture.texture, data, static_cast<unsigned int>(width), static_cast<unsigned int>(height));
     }*/
} // namespace LibOpenNFS::NFS4