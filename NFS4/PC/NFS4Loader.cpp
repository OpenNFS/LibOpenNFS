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
        track.trackTextureAssets = _ParseTextures(frdFile, track, trackOutPath);
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
            TrackBlock trackBlock(trackblockIdx, rawTrackBlockCenter, rawTrackBlock.header.unknown3, rawTrackBlock.header.nPositions,
                                  trackBlockNeighbourIds);

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

            // 4 OBJ Poly blocks
            for (uint32_t j = 0; j < 11; ++j) {
                if (rawTrackBlock.header.nobj[j].num > 0) {
                    // Iterate through objects in objpoly block up to num objects
                    for (uint32_t objectIdx = 0; objectIdx < rawTrackBlock.header.nobj[j].num; ++objectIdx) {
                        // Mesh Data
                        std::vector<uint32_t> vertexIndices;
                        std::vector<uint32_t> textureIndices;
                        std::vector<glm::vec2> uvs;
                        std::vector<glm::vec3> normals;
                        uint32_t accumulatedObjectFlags{0u};

                        // Get Polygons in object
                        std::vector<Polygon> objectPolygons{rawTrackBlock.polygonData.at(j)[objectIdx]};

                        for (auto &objectPolygon : objectPolygons) {
                            auto const &polygon{objectPolygon};

                            // Store into the track texture map if referenced by a polygon
                            if (!track.trackTextureAssets.contains(polygon.texture_id())) {
                                track.trackTextureAssets[polygon.texture_id()] =
                                    TrackTextureAsset(polygon.texture_id(), UINT32_MAX, UINT32_MAX, "", "");
                            }

                            /// Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture
                            // flags
                            TrackTextureAsset trackTextureAsset{track.trackTextureAssets.at(polygon.texture_id())};
                            std::vector<glm::vec2> uvs{{1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f},
                                                        {1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}};
                            std::vector<glm::vec2> transformedUVs{
                                trackTextureAsset.ScaleUVs(uvs, polygon.invert(), true, polygon.rotate())};
                            uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                            // Calculate the normal, as the provided data is a little suspect
                            glm::vec3 normal{Utils::CalculateQuadNormal(
                                rawTrackBlock.vertices[objectPolygon.vertex[0]], rawTrackBlock.vertices[objectPolygon.vertex[1]],
                                rawTrackBlock.vertices[objectPolygon.vertex[2]], rawTrackBlock.vertices[objectPolygon.vertex[3]])};

                            // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required
                            // per-vertex.
                            for (auto &quadToTriVertNumber : quadToTriVertNumbers) {
                                normals.emplace_back(normal);
                                vertexIndices.emplace_back(objectPolygon.vertex[quadToTriVertNumber]);
                                textureIndices.emplace_back(polygon.texture_id());
                            }

                            accumulatedObjectFlags |= polygon.texflags;
                        }
                        TrackGeometry trackBlockModel(trackBlockVerts, normals, uvs, textureIndices, vertexIndices, trackBlockShadingData,
                                                      rawTrackBlockCenter);
                        trackBlock.objects.emplace_back((j + 1) * (objectIdx + 1), EntityType::OBJ_POLY, trackBlockModel,
                                                        accumulatedObjectFlags);
                    }
                }
            }
            trackBlocks.push_back(trackBlock);
        }

        return trackBlocks;
    }

    std::map<uint32_t, TrackTextureAsset> Loader::_ParseTextures(FrdFile const &frdFile, Track &track,
                                                                 std::string const &trackOutPath) {
        std::map<uint32_t, TrackTextureAsset> textureAssetMap;
        size_t max_width{0}, max_height{0};

        // Load QFS texture information into ONFS texture objects
        for (auto &[textureId, textureAsset] : track.trackTextureAssets) {
            std::stringstream fileReference;
            std::stringstream alphaFileReference;

            // Find the maximum width and height, so we can avoid overestimating with blanket values (256x256) and
            // thereby scale UV's uneccesarily
            max_width = textureAsset.width > max_width ? textureAsset.width : max_width;
            max_height = textureAsset.height > max_height ? textureAsset.height : max_height;

            if (textureAsset.id >> 11) {
                fileReference << "../resources/sfx/" << std::setfill('0') << std::setw(4) << textureId + 9 << ".BMP";
                alphaFileReference << "../resources/sfx/" << std::setfill('0') << std::setw(4) << textureId + 9 << "-a.BMP";
            } else {
                fileReference << trackOutPath << "/" << track.name << "/textures/" << std::setfill('0') << std::setw(4) << textureId
                              << ".BMP";
                alphaFileReference << trackOutPath << "/" << track.name << "/textures/" << std::setfill('0') << std::setw(4) << textureId
                                   << "-a.BMP";
            }

            textureAsset.fileReference = fileReference.str();
            textureAsset.alphaFileReference = alphaFileReference.str();
            // TODO: Need to grab width and height of these references for texture scaling
        }

        // Now that maximum width/height is known, set the Max U/V for the texture
        for (auto &[id, textureAsset] : textureAssetMap) {
            // Attempt to remove potential for sampling texture from transparent area
            textureAsset.maxU = (static_cast<float>(textureAsset.width) / static_cast<float>(max_width)) - 0.005f;
            textureAsset.maxV = (static_cast<float>(textureAsset.height) / static_cast<float>(max_height)) - 0.005f;
        }

        return textureAssetMap;
    }

    std::vector<TrackVRoad> Loader::_ParseVirtualRoad(FrdFile const &frdFile) {
        std::vector<TrackVRoad> virtualRoad;

        for (uint16_t vroadIdx = 0; vroadIdx < frdFile.numVRoad; ++vroadIdx) {
            VRoadBlock vroad{frdFile.vroadBlocks.at(vroadIdx)};

            // Transform NFS3/4 coords into ONFS 3d space
            glm::vec3 position{Utils::FixedToFloat(vroad.refPt) * NFS4_SCALE_FACTOR};
            position.y += 0.2f;

            // Get VROAD right vector
            auto right{glm::vec3(vroad.right) / 128.f};
            auto forward{glm::vec3(vroad.forward)};
            auto normal{glm::vec3(vroad.normal)};

            glm::vec3 leftWall{((vroad.leftWall / 65536.0f) * NFS4_SCALE_FACTOR) * right};
            glm::vec3 rightWall{((vroad.rightWall / 65536.0f) * NFS4_SCALE_FACTOR) * right};

            virtualRoad.emplace_back(position, glm::vec3(0, 0, 0), normal, forward, right, leftWall, rightWall, vroad.unknown2[0]);
        }

        return virtualRoad;
    }
} // namespace LibOpenNFS::NFS4