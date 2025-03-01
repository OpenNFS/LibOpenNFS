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
        canPath = trackBasePath + "/" + "tr00.can";

        FrdFile frdFile;
        Shared::CanFile canFile;

        // Load FRD file to get track block specific data
        ASSERT(FrdFile::Load(frdPath, frdFile), "Could not load FRD file: " << frdPath);
        // Load camera intro/outro animation data
        ASSERT(Shared::CanFile::Load(canPath, canFile), "Could not load CAN file (camera animation): " << canPath);

        track.nBlocks = frdFile.nBlocks;
        track.cameraAnimation = canFile.animPoints;
        track.trackTextureAssets = _ParseTextures(track, trackOutPath);
        track.trackBlocks = _ParseFRDModels(frdFile, track);
        track.virtualRoad = _ParseVirtualRoad(frdFile);

        LogInfo("Track loaded successfully");

        return track;
    }

    Car::MetaData Loader::_ParseAssetData(FceFile const &fceFile, FedataFile const &fedataFile, NFSVersion version) {
        LogInfo("Parsing FCE File into ONFS Structures");
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

    std::map<uint32_t, TrackTextureAsset> Loader::_ParseTextures(Track const &track, std::string const &trackOutPath) {
        using namespace std::filesystem;

        // TODO: Hack :( OpenNFS needs to scale the UVs by the proportion of the textures size to the max sized texture on the track,
        // due to the usage of a texture array in the renderer. NFS4 doesn't encode the size of the texture inside the FRD file, hence
        // we need to grab the dimensions from the QFS file. As we rely on fshtool for QFS unpacking and don't have a dedicated QFS parser
        // we will 'dumbly' reuse the extraction utility function, and then parse the associated bitmap headers for width and height, ahead
        // of geometry parsing. This lets us scale the UVs directly at model creation time. This has the unfortunate affect of saddling
        // LibOpenNFS users with this redundant process, hence we'll IFDEF it to be active only for OpenNFS's usage.
        std::string const fullTrackOutPath{trackOutPath + track.name};
        ASSERT(TextureUtils::ExtractTrackTextures(track.basePath, track.name, track.nfsVersion, fullTrackOutPath),
               "Could not extract texture pack");

        // TODO: Grab these from the QFS directly instead of extracting...
        std::map<uint32_t, TrackTextureAsset> textureAssetMap;
        size_t max_width{0}, max_height{0};
        std::string const textureOutPath{trackOutPath + track.name + "/textures/"};

        for (recursive_directory_iterator iter(textureOutPath), end; iter != end; ++iter) {
            path texturePath{iter->path()};
            if (texturePath.extension().string() != ".BMP") {
                continue;
            }
            std::string textureFilename{texturePath.filename().string()};
            auto const textureId{std::atoi(textureFilename.substr(0, textureFilename.size() - 4).c_str())};
            auto const [width, height] = TextureUtils::GetBitmapDimensions(texturePath.string());

            // Find the maximum width and height, so we can avoid overestimating with blanket values (256x256) and
            // thereby scale UV's unnecessarily
            max_width = width > max_width ? width : max_width;
            max_height = height > max_height ? height : max_height;

            // Load QFS texture information into ONFS texture objects
            textureAssetMap[textureId] = TrackTextureAsset(textureId, width, height, texturePath.string(), "");
        }

        // Now that maximum width/height is known, set the Max U/V for the texture
        for (auto &[id, textureAsset] : textureAssetMap) {
            // Attempt to remove potential for sampling texture from transparent area
            textureAsset.maxU = (static_cast<float>(textureAsset.width) / static_cast<float>(max_width)) - 0.005f;
            textureAsset.maxV = (static_cast<float>(textureAsset.height) / static_cast<float>(max_height)) - 0.005f;
        }

        return textureAssetMap;
    }

    std::vector<TrackBlock> Loader::_ParseFRDModels(FrdFile const &frdFile, Track &track) {
        LogInfo("Parsing FRD file into ONFS GL structures");
        std::vector<TrackBlock> trackBlocks;
        trackBlocks.reserve(frdFile.nBlocks);

        for (uint32_t trackblockIdx = 0; trackblockIdx < frdFile.nBlocks; trackblockIdx++) {
            TrkBlock const &rawTrackBlock{frdFile.trackBlocks[trackblockIdx]};

            glm::vec3 rawTrackBlockCenter{rawTrackBlock.header.ptCentre * NFS4_SCALE_FACTOR};
            std::vector<uint32_t> trackBlockNeighbourIds;
            std::vector<glm::vec3> trackBlockVerts;
            std::vector<glm::vec4> trackBlockShadingData;

            // Get neighbouring block IDs
            for (auto &neighbourBlockData : rawTrackBlock.header.nbdData) {
                if (neighbourBlockData.blk == -1) {
                    break;
                }
                trackBlockNeighbourIds.emplace_back(neighbourBlockData.blk);
            }

            // Build the base OpenNFS trackblock, to hold all the geometry and virtual road data, lights, sounds etc.
            // for this portion of track
            TrackBlock trackBlock(trackblockIdx, rawTrackBlockCenter, 0, rawTrackBlock.header.nPositions, trackBlockNeighbourIds);

            // Light and sound sources
            for (uint32_t lightNum = 0; lightNum < rawTrackBlock.header.nLightsrc.num; ++lightNum) {
                glm::vec3 lightCenter{Utils::FixedToFloat(rawTrackBlock.lightsrc[lightNum].refpoint) * NFS4_SCALE_FACTOR};
                trackBlock.lights.emplace_back(lightNum, lightCenter, rawTrackBlock.lightsrc[lightNum].type);
            }
            for (uint32_t soundNum = 0; soundNum < rawTrackBlock.header.nSoundsrc.num; ++soundNum) {
                glm::vec3 soundCenter{Utils::FixedToFloat(rawTrackBlock.soundsrc[soundNum].refpoint) * NFS4_SCALE_FACTOR};
                trackBlock.sounds.emplace_back(soundNum, soundCenter, rawTrackBlock.soundsrc[soundNum].type);
            }

            // Get Trackblock roadVertices and per-vertex shading data
            for (uint32_t vertIdx = 0; vertIdx < rawTrackBlock.header.nObjectVert; ++vertIdx) {
                trackBlockVerts.emplace_back((rawTrackBlock.vertices[vertIdx] * NFS4_SCALE_FACTOR) - rawTrackBlockCenter);
                trackBlockShadingData.emplace_back(TextureUtils::ShadingDataToVec4(rawTrackBlock.shadingVertices[vertIdx]));
            }

            // High-Poly Data is in block 4
            /*for (auto const &polygon : rawTrackBlock.polygonData.at(4)) {
                // Mesh Data
                std::vector<uint32_t> vertexIndices;
                std::vector<uint32_t> textureIndices;
                std::vector<glm::vec3> normals;
                uint32_t accumulatedObjectFlags{0u};

                // Store into the track texture map if referenced by a polygon
                if (!track.trackTextureAssets.contains(polygon.texture_id())) {
                    track.trackTextureAssets[polygon.texture_id()] =
                        TrackTextureAsset(polygon.texture_id(), UINT32_MAX, UINT32_MAX, "", "");
                }

                /// Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture
                // flags
                TrackTextureAsset trackTextureAsset{track.trackTextureAssets.at(polygon.texture_id())};
                std::vector<glm::vec2> uvs{{1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}};
                std::vector<glm::vec2> transformedUVs{trackTextureAsset.ScaleUVs(uvs, polygon.invert(), true, polygon.rotate())};
                uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                // Calculate the normal, as the provided data is a little suspect
                glm::vec3 normal{
                    Utils::CalculateQuadNormal(rawTrackBlock.vertices[polygon.vertex[0]], rawTrackBlock.vertices[polygon.vertex[1]],
                                               rawTrackBlock.vertices[polygon.vertex[2]], rawTrackBlock.vertices[polygon.vertex[3]])};

                // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                for (auto &quadToTriVertNumber : quadToTriVertNumbers) {
                    normals.emplace_back(normal);
                    vertexIndices.emplace_back(polygon.vertex[quadToTriVertNumber]);
                    textureIndices.emplace_back(polygon.texture_id());
                }

                accumulatedObjectFlags |= polygon.texflags;

                TrackGeometry trackBlockModel(trackBlockVerts, normals, uvs, textureIndices, vertexIndices, trackBlockShadingData,
                                              rawTrackBlockCenter);
                trackBlock.track.emplace_back(0, EntityType::OBJ_POLY, trackBlockModel, accumulatedObjectFlags);
            }*/

            /*for (auto &extraObject : rawTrackBlock.extraObjects) {
                // Iterate through objects in objpoly block up to num objects
                for (uint32_t objectIdx = 0; objectIdx < extraObject.nObjects; ++objectIdx) {
                    auto const &object{extraObject.objectBlocks.at(objectIdx)};
                    auto const &objectHeader{extraObject.objectHeaders.at(objectIdx)};

                    // Mesh Data
                    std::vector<uint32_t> vertexIndices;
                    std::vector<uint32_t> textureIndices;
                    std::vector<glm::vec2> uvs;
                    std::vector<glm::vec3> normals;
                    uint32_t accumulatedObjectFlags{0u};

                    // Get Polygons in object
                    LogInfo("Parsing TrackBlock: %d Object: %d NumPolys: %d", trackblockIdx, objectIdx, object.polygons.size());

                    for (size_t polyIdx = 0; polyIdx < objectHeader.nPolygons; ++polyIdx) {
                        auto const &polygon{object.polygons.at(polyIdx)};

                        // Store into the track texture map if referenced by a polygon
                        if (!track.trackTextureAssets.contains(polygon.texture_id())) {
                            track.trackTextureAssets[polygon.texture_id()] =
                                TrackTextureAsset(polygon.texture_id(), UINT32_MAX, UINT32_MAX, "", "");
                        }

                        /// Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture
                        // flags
                        TrackTextureAsset trackTextureAsset{track.trackTextureAssets.at(polygon.texture_id())};
                        std::vector<glm::vec2> uvs{{1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}};
                        std::vector<glm::vec2> transformedUVs{trackTextureAsset.ScaleUVs(uvs, polygon.invert(), true, polygon.rotate())};
                        uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                        // Calculate the normal, as the provided data is a little suspect
                        glm::vec3 normal{Utils::CalculateQuadNormal(
                            rawTrackBlock.vertices[polygon.vertex[0]], rawTrackBlock.vertices[polygon.vertex[1]],
                            rawTrackBlock.vertices[polygon.vertex[2]], rawTrackBlock.vertices[polygon.vertex[3]])};

                        // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                        for (auto &quadToTriVertNumber : quadToTriVertNumbers) {
                            normals.emplace_back(normal);
                            vertexIndices.emplace_back(polygon.vertex[quadToTriVertNumber]);
                            textureIndices.emplace_back(polygon.texture_id());
                        }

                        accumulatedObjectFlags |= polygon.texflags;
                    }
                    TrackGeometry trackBlockModel(trackBlockVerts, normals, uvs, textureIndices, vertexIndices, trackBlockShadingData,
                                                  rawTrackBlockCenter);
                    trackBlock.objects.emplace_back(objectIdx, EntityType::XOBJ, trackBlockModel, accumulatedObjectFlags);
                }
            }*/

            // Road Mesh data
            std::vector<glm::vec3> roadVertices;
            std::vector<glm::vec4> roadShadingData;
            std::vector<uint32_t> vertexIndices;
            std::vector<uint32_t> textureIndices;
            std::vector<glm::vec2> uvs;
            std::vector<glm::vec3> normals;
            uint32_t accumulatedObjectFlags{0u};

            for (uint32_t vertIdx = 0; vertIdx < rawTrackBlock.header.nVertices; ++vertIdx) {
                roadVertices.emplace_back((rawTrackBlock.vertices[vertIdx] * NFS4_SCALE_FACTOR) - rawTrackBlockCenter);
                roadShadingData.emplace_back(TextureUtils::ShadingDataToVec4(rawTrackBlock.shadingVertices[vertIdx]));
            }
            // Get indices from Chunk 4 and 5 for High Res polys, Chunk 6 for Road Lanes
            for (uint32_t lodChunkIdx = 4; lodChunkIdx <= 6; lodChunkIdx++) {
                // If there are no lane markers in the lane chunk, skip
                if ((lodChunkIdx == 6) && (rawTrackBlock.header.nVertices <= rawTrackBlock.header.nHiResVert)) {
                    continue;
                }

                // Get the polygon data for this road section
                auto const &chunkPolygonData{rawTrackBlock.polygonData.at(lodChunkIdx)};

                for (uint32_t polyIdx = 0; polyIdx < rawTrackBlock.header.sz[lodChunkIdx]; polyIdx++) {
                    auto const &polygon{chunkPolygonData[polyIdx]};
                    // Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture flags
                    if (!track.trackTextureAssets.contains(polygon.texture_id())) {
                        track.trackTextureAssets[polygon.texture_id()] = TrackTextureAsset(polygon.texture_id(), 64, 64, "", "");
                    }
                    TrackTextureAsset trackTextureAsset{track.trackTextureAssets.at(polygon.texture_id())};
                    std::vector<glm::vec2> uv_temp{{1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}};
                    std::vector<glm::vec2> transformedUVs{trackTextureAsset.ScaleUVs(uv_temp, polygon.invert(), true, polygon.rotate())};
                    uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                    glm::vec3 normal =
                        Utils::CalculateQuadNormal(rawTrackBlock.vertices[polygon.vertex[0]], rawTrackBlock.vertices[polygon.vertex[1]],
                                                   rawTrackBlock.vertices[polygon.vertex[2]], rawTrackBlock.vertices[polygon.vertex[3]]);

                    // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                    for (auto &quadToTriVertNumber : quadToTriVertNumbers) {
                        normals.emplace_back(normal);
                        vertexIndices.emplace_back(polygon.vertex[quadToTriVertNumber]);
                        textureIndices.emplace_back(polygon.texture_id());
                    }

                    accumulatedObjectFlags |= polygon.texflags;
                }
                auto roadModel{
                    TrackGeometry(roadVertices, normals, uvs, textureIndices, vertexIndices, roadShadingData, rawTrackBlockCenter)};
                if (lodChunkIdx == 6) {
                    trackBlock.lanes.emplace_back(-1, EntityType::LANE, roadModel, accumulatedObjectFlags);
                } else {
                    trackBlock.track.emplace_back(-1, EntityType::ROAD, roadModel, accumulatedObjectFlags);
                }
            }

            trackBlocks.push_back(trackBlock);
        }

        return trackBlocks;
    }

    std::vector<TrackVRoad> Loader::_ParseVirtualRoad(FrdFile const &frdFile) {
        std::vector<TrackVRoad> virtualRoad;

        for (uint16_t vroadIdx = 0; vroadIdx < frdFile.numVRoad; ++vroadIdx) {
            VRoadBlock vroad{frdFile.vroadBlocks.at(vroadIdx)};

            // Transform NFS3/4 coords into ONFS 3d space
            glm::vec3 position{vroad.refPt * NFS4_SCALE_FACTOR};
            position.y += 0.2f;
            glm::vec3 leftWall{(vroad.leftWall * NFS4_SCALE_FACTOR) * vroad.right};
            glm::vec3 rightWall{(vroad.rightWall * NFS4_SCALE_FACTOR) * vroad.right};

            virtualRoad.emplace_back(position, glm::vec3(0, 0, 0), vroad.normal, vroad.forward, vroad.right, leftWall, rightWall, vroad.unknown2[0]);
        }

        return virtualRoad;
    }
} // namespace LibOpenNFS::NFS4