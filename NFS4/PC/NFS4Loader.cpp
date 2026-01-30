#include "NFS4Loader.h"

#include "FCE/FceFile.h"
#include "FRD/AnimBlock.h"

#include <../../Shared/VIV/VivArchive.h>
#include <Common/Logging.h>
#include <Common/Utils.h>
#include <Shared/FSH/FshArchive.h>

#include <sstream>

namespace LibOpenNFS::NFS4 {
    Car Loader::LoadCar(std::string const &carBasePath, std::string const &carOutPath, NFSVersion version) {
        LogInfo("Loading NFS4 car from %s into %s", carBasePath.c_str(), carOutPath.c_str());

        std::filesystem::path p(carBasePath);
        std::string carName = p.filename().replace_extension("").string();

        std::stringstream vivPath, fcePath, fshPath, fedataPath, carpPath;
        vivPath << carBasePath;
        fcePath << carOutPath;
        fedataPath << carOutPath << "/fedata.eng";
        carpPath << carOutPath << "/carp.txt";

        if (version == NFSVersion::NFS_4) {
            vivPath << "/car.viv";
            fcePath << "/car.fce";
        } else {
            // MCO
            vivPath << ".viv";
            fcePath << "/part.fce";
            fshPath << "../resources/MCO/Data/skins/" << carName.substr(0, carName.size() - 2) << "dec.fsh";
        }

        Shared::VivArchive vivFile;
        FceFile fceFile;
        FedataFile fedataFile;
        Shared::CarpFile carpFile;

        Car::PhysicsData carPhysicsData;

        if (std::filesystem::exists(fcePath.str()) && std::filesystem::exists(fedataPath.str()) && std::filesystem::exists(carpPath.str())) {
            LogInfo("VIV has already been extracted to %s, skipping", carOutPath.c_str());
        } else {
            ASSERT(Shared::VivArchive::Load(vivPath.str(), vivFile), "Could not open VIV file: " << vivPath.str());
            ASSERT(Shared::VivArchive::Extract(carOutPath, vivFile),
                   "Could not extract VIV file: " << vivPath.str() << "to: " << carOutPath);
        }
        ASSERT(NFS4::FceFile::Load(fcePath.str(), fceFile), "Could not load FCE file: " << fcePath.str());
        if (!FedataFile::Load(fedataPath.str(), fedataFile)) {
            LogWarning("Could not load FeData file: %s", fedataPath.str().c_str());
        }
        if (Shared::CarpFile::Load(carpPath.str(), carpFile)) {
            carPhysicsData = _ParsePhysicsData(carpFile);
        } else {
            LogWarning("Could not load carp.txt file: %s", carpPath.str().c_str());
        }

        if (version == NFSVersion::MCO) {
            if (std::filesystem::exists(fshPath.str())) {
                TextureUtils::ExtractQFS(fshPath.str(), carOutPath + "/Textures/");
            } else {
                LogInfo("Can't find MCO car texture at %s (More work needed to identify when certain fsh's are used)",
                        fshPath.str().c_str());
            }
        }

        Car::MetaData const carData{_ParseAssetData(fceFile, fedataFile, version)};

        return Car(carData, version, carName, carPhysicsData);
    }

    Track Loader::LoadTrack(std::string const &trackBasePath) {
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
        track.trackTextureAssets = _ParseTextures(track);
        std::tie(track.trackBlocks, track.globalObjects) = _ParseFRDModels(frdFile, track);
        track.virtualRoad = _ParseVirtualRoad(frdFile);

        LogInfo("Track loaded successfully");

        return track;
    }

    FedataFile Loader::LoadCarMenuData(std::string const &carBasePath, std::string const &carOutPath, NFSVersion version) {
         LogInfo("Loading NFS4 car menu data from %s into %s", carBasePath.c_str(), carOutPath.c_str());

        std::filesystem::path p(carBasePath);
        std::string carName = p.filename().replace_extension("").string();
        std::string const fedataFileName = "fedata.eng";

        std::stringstream vivPath, fedataPath;
        vivPath << carBasePath;
        fedataPath << carOutPath << "/" << fedataFileName;

        if (version == NFSVersion::NFS_4) {
            vivPath << "/car.viv";
        } else {
            // MCO
            vivPath << ".viv";
        }

        Shared::VivArchive vivFile;
        FceFile fceFile;
        FedataFile fedataFile;

        if (std::filesystem::exists(fedataPath.str())) {
            LogInfo("Fedata file has already been extracted to %s, skipping", carOutPath.c_str());
        } else {
            ASSERT(Shared::VivArchive::Load(vivPath.str(), vivFile), "Could not open VIV file: " << vivPath.str());
            ASSERT(Shared::VivArchive::ExtractFile(carOutPath, vivFile, fedataFileName),
                   "Could not extract fedata file from VIV file: " << vivPath.str() << "to: " << carOutPath);
        }
        if (!FedataFile::Load(fedataPath.str(), fedataFile)) {
            LogWarning("Could not load FeData file: %s", fedataPath.str().c_str());
        }

        return fedataFile;
    }

    Car::MetaData Loader::_ParseAssetData(FceFile const &fceFile, FedataFile const &fedataFile, NFSVersion version) {
        LogInfo("Parsing FCE File into ONFS Structures");
        Car::MetaData carMetadata;

        // Go get car metadata from FEDATA
        carMetadata.name = fedataFile.menuName;

        // Grab colours
        for (uint32_t colourIdx{0}; colourIdx < fceFile.nColours; ++colourIdx) {
            if (fceFile.nColours == 1) {
                break;
            }
            auto [Hp, Sp, Bp, Tp] = fceFile.primaryColours[colourIdx];
            auto [Hs, Ss, Bs, Ts] = fceFile.secondaryColours[colourIdx];
            Car::Colour originalPrimaryColour(fedataFile.primaryColourNames[colourIdx], TextureUtils::HSLToRGB(glm::vec4(Hp, Sp, Bp, Tp)),
                                              TextureUtils::HSLToRGB(glm::vec4(Hs, Ss, Bs, Ts)));
            carMetadata.colours.emplace_back(originalPrimaryColour);
        }

        for (uint32_t dummyIdx{0}; dummyIdx < fceFile.nDummies; ++dummyIdx) {
            Car::Dummy dummy(fceFile.dummyObjectInfo[dummyIdx].data, fceFile.dummyCoords[dummyIdx] * CAR_SCALE_FACTOR);
            carMetadata.dummies.emplace_back(dummy);
        }

        for (uint32_t partIdx{0}; partIdx < fceFile.nParts; ++partIdx) {
            std::vector<uint32_t> indices;
            std::vector<uint32_t> polygonFlags;
            std::vector<glm::vec3> vertices;
            std::vector<glm::vec3> normals;
            std::vector<glm::vec2> uvs;

            std::string part_name(fceFile.partNames[partIdx]);
            glm::vec3 center{fceFile.partCoords[partIdx] * CAR_SCALE_FACTOR};
            FceFile::CarPart const &part{fceFile.carParts[partIdx]};

            vertices.reserve(fceFile.partNumVertices[partIdx]);
            normals.reserve(fceFile.partNumVertices[partIdx]);
            polygonFlags.reserve(fceFile.partNumTriangles[partIdx] * 3);
            indices.reserve(fceFile.partNumTriangles[partIdx] * 3);
            uvs.reserve(fceFile.partNumTriangles[partIdx] * 3);

            for (uint32_t vert_Idx{0}; vert_Idx < fceFile.partNumVertices[partIdx]; ++vert_Idx) {
                vertices.emplace_back(part.vertices[vert_Idx] * CAR_SCALE_FACTOR);
            }
            for (uint32_t normal_Idx{0}; normal_Idx < fceFile.partNumVertices[partIdx]; ++normal_Idx) {
                normals.emplace_back(part.normals[normal_Idx] * CAR_SCALE_FACTOR);
            }
            for (uint32_t tri_Idx = {0}; tri_Idx < fceFile.partNumTriangles[partIdx]; ++tri_Idx) {
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

    Car::PhysicsData Loader::_ParsePhysicsData(Shared::CarpFile const &carpFile) {
        Car::PhysicsData physicsData;

        // Old fields (for backwards compatibility with Bullet physics)
        physicsData.mass = carpFile.mass;
        physicsData.maxSpeed = carpFile.topSpeedCap * 3.6f; // topSpeedCap is in m/s
        physicsData.suspensionStiffness = carpFile.suspensionStiffness * 750.f;
        physicsData.maxBreakingForce = carpFile.maximumBrakingDeceleration * 3.6f;

        // NFS3/NFS4 carp.txt fields
        physicsData.serialNumber = carpFile.serialNumber;
        physicsData.carClassification = carpFile.carClassification;
        physicsData.numberOfGearsManual = carpFile.numberOfGearsManual;
        physicsData.numberOfGearsAutomatic = carpFile.numberOfGearsAutomatic;
        physicsData.gearShiftDelay = carpFile.gearShiftDelay;
        physicsData.shiftBlipInRpm = carpFile.shiftBlipInRpm;
        physicsData.brakeBlipInRpm = carpFile.brakeBlipInRpm;
        physicsData.velocityToRpmRatioManual = carpFile.velocityToRpmRatioManual;
        physicsData.velocityToRpmRatioAutomatic = carpFile.velocityToRpmRatioAutomatic;
        physicsData.gearRatiosManual = carpFile.gearRatiosManual;
        physicsData.gearRatiosAutomatic = carpFile.gearRatiosAutomatic;
        physicsData.gearEfficiencyManual = carpFile.gearEfficiencyManual;
        physicsData.gearEfficiencyAutomatic = carpFile.gearEfficiencyAutomatic;
        physicsData.torqueCurve = carpFile.torqueCurve;
        physicsData.finalGearManual = carpFile.finalGearManual;
        physicsData.finalGearAutomatic = carpFile.finalGearAutomatic;
        physicsData.engineMinimumRpm = carpFile.engineMinimumRpm;
        physicsData.engineRedlineInRpm = carpFile.engineRedlineInRpm;
        physicsData.maximumVelocityOfCar = carpFile.maximumVelocityOfCar;
        physicsData.topSpeedCap = carpFile.topSpeedCap;
        physicsData.frontDriveRatio = carpFile.frontDriveRatio;
        physicsData.usesAntilockBrakeSystem = carpFile.usesAntilockBrakeSystem;
        physicsData.maximumBrakingDeceleration = carpFile.maximumBrakingDeceleration;
        physicsData.frontBiasBrakeRatio = carpFile.frontBiasBrakeRatio;
        physicsData.gasIncreasingCurve = carpFile.gasIncreasingCurve;
        physicsData.gasDecreasingCurve = carpFile.gasDecreasingCurve;
        physicsData.brakeIncreasingCurve = carpFile.brakeIncreasingCurve;
        physicsData.brakeDecreasingCurve = carpFile.brakeDecreasingCurve;
        physicsData.wheelBase = carpFile.wheelBase;
        physicsData.frontGripBias = carpFile.frontGripBias;
        physicsData.powerSteering = carpFile.powerSteering;
        physicsData.minimumSteeringAcceleration = carpFile.minimumSteeringAcceleration;
        physicsData.turnInRamp = carpFile.turnInRamp;
        physicsData.turnOutRamp = carpFile.turnOutRamp;
        physicsData.lateralAccelerationGripMultiplier = carpFile.lateralAccelerationGripMultiplier;
        physicsData.aerodynamicDownforceMultiplier = carpFile.aerodynamicDownforceMultiplier;
        physicsData.gasOffFactor = carpFile.gasOffFactor;
        physicsData.gTransferFactor = carpFile.gTransferFactor;
        physicsData.turningCircleRadius = carpFile.turningCircleRadius;
        physicsData.tireSpecsFront = carpFile.tireSpecsFront;
        physicsData.tireSpecsRear = carpFile.tireSpecsRear;
        physicsData.tireWear = carpFile.tireWear;
        physicsData.slideMultiplier = carpFile.slideMultiplier;
        physicsData.spinVelocityCap = carpFile.spinVelocityCap;
        physicsData.slideVelocityCap = carpFile.slideVelocityCap;
        physicsData.slideAssistanceFactor = carpFile.slideAssistanceFactor;
        physicsData.pushFactor = carpFile.pushFactor;
        physicsData.lowTurnFactor = carpFile.lowTurnFactor;
        physicsData.highTurnFactor = carpFile.highTurnFactor;
        physicsData.pitchRollFactor = carpFile.pitchRollFactor;
        physicsData.roadBumpinessFactor = carpFile.roadBumpinessFactor;
        physicsData.spoilerFunctionType = carpFile.spoilerFunctionType;
        physicsData.spoilerActivationSpeed = carpFile.spoilerActivationSpeed;
        physicsData.gradualTurnCutoff = carpFile.gradualTurnCutoff;
        physicsData.mediumTurnCutoff = carpFile.mediumTurnCutoff;
        physicsData.sharpTurnCutoff = carpFile.sharpTurnCutoff;
        physicsData.mediumTurnSpeedModifier = carpFile.mediumTurnSpeedModifier;
        physicsData.sharpTurnSpeedModifier = carpFile.sharpTurnSpeedModifier;
        physicsData.extremeTurnSpeedModifier = carpFile.extremeTurnSpeedModifier;
        physicsData.subdivideLevel = carpFile.subdivideLevel;
        physicsData.cameraArm = carpFile.cameraArm;
        physicsData.bodyDamage = carpFile.bodyDamage;
        physicsData.engineDamage = carpFile.engineDamage;
        physicsData.suspensionDamage = carpFile.suspensionDamage;
        physicsData.engineTuning = carpFile.engineTuning;
        physicsData.breakBalance = carpFile.breakBalance;
        physicsData.steeringSpeed = carpFile.steeringSpeed;
        physicsData.gearRatFactor = carpFile.gearRatFactor;
        physicsData.aeroFactor = carpFile.aeroFactor;
        physicsData.tireFactor = carpFile.tireFactor;
        physicsData.aiAcc0AccelerationTable = carpFile.aiAcc0AccelerationTable;
        physicsData.aiAcc1AccelerationTable = carpFile.aiAcc1AccelerationTable;
        physicsData.aiAcc2AccelerationTable = carpFile.aiAcc2AccelerationTable;
        physicsData.aiAcc3AccelerationTable = carpFile.aiAcc3AccelerationTable;
        physicsData.aiAcc4AccelerationTable = carpFile.aiAcc4AccelerationTable;
        physicsData.aiAcc5AccelerationTable = carpFile.aiAcc5AccelerationTable;
        physicsData.aiAcc6AccelerationTable = carpFile.aiAcc6AccelerationTable;
        physicsData.aiAcc7AccelerationTable = carpFile.aiAcc7AccelerationTable;
        // NFS4 specific
        physicsData.understeerGradient = carpFile.understeerGradient;

        return physicsData;
    }

    std::map<uint32_t, TrackTextureAsset> Loader::_ParseTextures(Track const &track) {
        Shared::FshArchive archive;
        ASSERT(archive.Load(track.texturePath, true),
               "Failed to load texture archive: " << track.texturePath << " - " << archive.LastError());

        std::map<uint32_t, TrackTextureAsset> textureAssetMap;
        size_t max_width{0}, max_height{0};

        uint32_t texId{0};
        for (auto const &tex : archive.Textures()) {
            // Find the maximum width and height for UV scaling
            max_width = tex.Width() > max_width ? tex.Width() : max_width;
            max_height = tex.Height() > max_height ? tex.Height() : max_height;

            // Get pixel data directly from FSH as RGBA
            textureAssetMap[texId] = TrackTextureAsset(texId, tex.Width(), tex.Height(), tex.ToRGBA());
            texId++;
        }

        // Now that maximum width/height is known, set the Max U/V for the texture
        for (auto &[id, textureAsset] : textureAssetMap) {
            // Attempt to remove potential for sampling texture from transparent area
            textureAsset.maxU = (static_cast<float>(textureAsset.width) / static_cast<float>(max_width)) - 0.005f;
            textureAsset.maxV = (static_cast<float>(textureAsset.height) / static_cast<float>(max_height)) - 0.005f;
        }

        return textureAssetMap;
    }

    std::pair<std::vector<TrackBlock>, std::vector<TrackEntity>> Loader::_ParseFRDModels(FrdFile const &frdFile, Track &track) {
        LogInfo("Parsing FRD file into ONFS GL structures");
        std::vector<TrackBlock> trackBlocks;
        trackBlocks.reserve(frdFile.nBlocks);
        std::vector<TrackEntity> globalObjects;
        uint32_t vroadCount{0};

        for (uint32_t trackblockIdx{0}; trackblockIdx < frdFile.nBlocks; ++trackblockIdx) {
            TrkBlock const &rawTrackBlock{frdFile.trackBlocks[trackblockIdx]};

            glm::vec3 rawTrackBlockCenter{rawTrackBlock.header.ptCentre * TRACK_SCALE_FACTOR};
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
            TrackBlock trackBlock(trackblockIdx, rawTrackBlockCenter, vroadCount, rawTrackBlock.header.nPositions, trackBlockNeighbourIds);
            vroadCount += rawTrackBlock.header.nPositions;

            // Light and sound sources
            for (uint32_t lightNum{0}; lightNum < rawTrackBlock.header.nLightsrc.num; ++lightNum) {
                glm::vec3 lightCenter{Utils::FixedToFloat(rawTrackBlock.lightsrc[lightNum].refpoint) * TRACK_SCALE_FACTOR};
                trackBlock.lights.emplace_back(lightNum, lightCenter, rawTrackBlock.lightsrc[lightNum].type);
            }
            for (uint32_t soundNum{0}; soundNum < rawTrackBlock.header.nSoundsrc.num; ++soundNum) {
                glm::vec3 soundCenter{Utils::FixedToFloat(rawTrackBlock.soundsrc[soundNum].refpoint) * TRACK_SCALE_FACTOR};
                trackBlock.sounds.emplace_back(soundNum, soundCenter, rawTrackBlock.soundsrc[soundNum].type);
            }

            // Get Trackblock roadVertices and per-vertex shading data
            for (uint32_t vertIdx{0}; vertIdx < rawTrackBlock.header.nObjectVert; ++vertIdx) {
                trackBlockVerts.emplace_back((rawTrackBlock.vertices[vertIdx] * TRACK_SCALE_FACTOR) - rawTrackBlockCenter);
                trackBlockShadingData.emplace_back(TextureUtils::ShadingDataToVec4(rawTrackBlock.shadingVertices[vertIdx]));
            }

            // ExtraObjects
            for (auto &extraObject : rawTrackBlock.extraObjects) {
                // Iterate through objects in objpoly block up to num objects
                for (uint32_t objectIdx{0}; objectIdx < extraObject.nObjects; ++objectIdx) {
                    // Mesh Data
                    std::vector<glm::vec3> extraObjectVerts;
                    std::vector<glm::vec4> extraObjectShadingData;
                    std::vector<uint32_t> vertexIndices;
                    std::vector<uint32_t> textureIndices;
                    std::vector<glm::vec2> xobj_uvs;
                    std::vector<glm::vec3> normals;
                    uint32_t accumulatedObjectFlags{0u};

                    // Get the Extra object data for this trackblock object from the global xobj table
                    auto const &object{extraObject.objectBlocks.at(objectIdx)};
                    auto const &objectHeader{extraObject.objectHeaders.at(objectIdx)};

                    for (uint32_t vertIdx{0}; vertIdx < objectHeader.nVertices; ++vertIdx) {
                        extraObjectVerts.emplace_back(object->vertices[vertIdx] * TRACK_SCALE_FACTOR);
                        extraObjectShadingData.emplace_back(TextureUtils::ShadingDataToVec4(object->shadingVertices[vertIdx]));
                    }

                    for (uint32_t polyIdx{0}; polyIdx < objectHeader.nPolygons; ++polyIdx) {
                        auto const &polygon{object->polygons.at(polyIdx)};

                        // Store into the track texture map if referenced by a polygon
                        if (!track.trackTextureAssets.contains(polygon.texture_id())) {
                            LogWarning("Texture: %d doesn't exist in loaded texture map, dummying", polygon.texture_id());
                            track.trackTextureAssets[polygon.texture_id()] =
                                TrackTextureAsset(polygon.texture_id(), UINT32_MAX, UINT32_MAX, "", "");
                        }

                        /// Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture
                        // flags
                        TrackTextureAsset trackTextureAsset{track.trackTextureAssets.at(polygon.texture_id())};
                        std::vector<glm::vec2> temp_uvs{{1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}};
                        std::vector<glm::vec2> transformedUVs{trackTextureAsset.ScaleUVs(
                            temp_uvs, false, !polygon.invert(), polygon.rotate(), polygon.mirror_x(), polygon.mirror_y())};
                        xobj_uvs.insert(xobj_uvs.end(), transformedUVs.begin(), transformedUVs.end());

                        glm::vec3 const normal{
                            Utils::CalculateQuadNormal(extraObjectVerts[polygon.vertex[0]], extraObjectVerts[polygon.vertex[1]],
                                                       extraObjectVerts[polygon.vertex[2]], extraObjectVerts[polygon.vertex[3]])};

                        // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required
                        // per-vertex.
                        for (auto &quadToTriVertNumber : quadToTriVertNumbers) {
                            normals.emplace_back(normal);
                            vertexIndices.emplace_back(polygon.vertex[quadToTriVertNumber]);
                            textureIndices.emplace_back(polygon.texture_id());
                        }

                        accumulatedObjectFlags |= polygon.texflags;
                    }
                    glm::vec3 extraObjectCenter{objectHeader.pt * TRACK_SCALE_FACTOR};
                    auto extraObjectModel{TrackGeometry(extraObjectVerts, normals, xobj_uvs, textureIndices, vertexIndices,
                                                        extraObjectShadingData, extraObjectCenter)};
                    trackBlock.objects.emplace_back(objectIdx, EntityType::XOBJ, extraObjectModel, accumulatedObjectFlags);
                }
            }

            // Road Mesh data
            std::vector<glm::vec3> roadVertices;
            std::vector<glm::vec4> roadShadingData;
            std::vector<uint32_t> vertexIndices;
            std::vector<uint32_t> textureIndices;
            std::vector<glm::vec2> uvs;
            std::vector<glm::vec3> normals;
            uint32_t accumulatedObjectFlags{0u};

            for (uint32_t vertIdx{0}; vertIdx < rawTrackBlock.header.nVertices; ++vertIdx) {
                roadVertices.emplace_back((rawTrackBlock.vertices[vertIdx] * TRACK_SCALE_FACTOR) - rawTrackBlockCenter);
                roadShadingData.emplace_back(TextureUtils::ShadingDataToVec4(rawTrackBlock.shadingVertices[vertIdx]));
            }

            // Get indices from Chunks 4+ for High Res polys, Chunk 6 for Road Lanes
            for (uint32_t lodChunkIdx = PolygonChunkType::HIGH_RES_TRACK; lodChunkIdx <= PolygonChunkType::HIGH_RES_MISC4; ++lodChunkIdx) {
                // Get the polygon data for this road section
                auto const &chunkPolygonData{rawTrackBlock.polygonData.at(lodChunkIdx)};

                for (uint32_t polyIdx{0}; polyIdx < rawTrackBlock.header.sz[lodChunkIdx]; ++polyIdx) {
                    auto const &polygon{chunkPolygonData[polyIdx]};
                    // Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture flags
                    if (!track.trackTextureAssets.contains(polygon.texture_id())) {
                        track.trackTextureAssets[polygon.texture_id()] = TrackTextureAsset(polygon.texture_id(), 64, 64, "", "");
                    }
                    TrackTextureAsset trackTextureAsset{track.trackTextureAssets.at(polygon.texture_id())};
                    std::vector<glm::vec2> temp_uvs{{1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}};
                    std::vector<glm::vec2> transformedUVs{trackTextureAsset.ScaleUVs(temp_uvs, false, !polygon.invert(), polygon.rotate(),
                                                                                     polygon.mirror_x(), polygon.mirror_y())};
                    uvs.insert(uvs.end(), transformedUVs.begin(), transformedUVs.end());

                    glm::vec3 const normal{
                        Utils::CalculateQuadNormal(rawTrackBlock.vertices[polygon.vertex[0]], rawTrackBlock.vertices[polygon.vertex[1]],
                                                   rawTrackBlock.vertices[polygon.vertex[2]], rawTrackBlock.vertices[polygon.vertex[3]])};

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
                if (lodChunkIdx == PolygonChunkType::LANES) {
                    trackBlock.lanes.emplace_back(-1, EntityType::LANE, roadModel, accumulatedObjectFlags);
                } else {
                    trackBlock.track.emplace_back(-1, EntityType::ROAD, roadModel, accumulatedObjectFlags);
                }
            }

            trackBlocks.push_back(trackBlock);
        }

        // Global Objects
        for (size_t globalObjBlockIdx{0}; globalObjBlockIdx < frdFile.globalObjects.size(); ++globalObjBlockIdx) {
            for (auto &globalObject : frdFile.globalObjects) {
                // Iterate through objects in objpoly block up to num objects
                for (uint32_t objectIdx{0}; objectIdx < globalObject.nObjects; ++objectIdx) {
                    // Mesh Data
                    std::vector<glm::vec3> extraObjectVerts;
                    std::vector<glm::vec4> extraObjectShadingData;
                    std::vector<uint32_t> vertexIndices;
                    std::vector<uint32_t> textureIndices;
                    std::vector<glm::vec2> xobj_uvs;
                    std::vector<glm::vec3> normals;
                    uint32_t accumulatedObjectFlags{0u};

                    // Get the Extra object data for this trackblock object from the global xobj table
                    auto const &object{globalObject.objectBlocks.at(objectIdx)};
                    auto const &objectHeader{globalObject.objectHeaders.at(objectIdx)};

                    std::vector<AnimKeyframe> AnimKeyframes;
                    uint16_t animDelay;
                    if (objectHeader.type == XObjHeader::Type::ANIMATED) {
                        auto animBlock = static_cast<AnimBlock const *>(object.get());
                        AnimKeyframes = animBlock->keyframes;
                        animDelay = animBlock->delay;
                    }

                    for (uint32_t vertIdx{0}; vertIdx < objectHeader.nVertices; ++vertIdx) {
                        extraObjectVerts.emplace_back(object->vertices[vertIdx] * TRACK_SCALE_FACTOR);
                        extraObjectShadingData.emplace_back(TextureUtils::ShadingDataToVec4(object->shadingVertices[vertIdx]));
                    }

                    for (uint32_t polyIdx{0}; polyIdx < objectHeader.nPolygons; ++polyIdx) {
                        auto const &polygon{object->polygons.at(polyIdx)};

                        // Store into the track texture map if referenced by a polygon
                        if (!track.trackTextureAssets.contains(polygon.texture_id())) {
                            LogWarning("Texture: %d doesn't exist in loaded texture map, dummying", polygon.texture_id());
                            track.trackTextureAssets[polygon.texture_id()] =
                                TrackTextureAsset(polygon.texture_id(), UINT32_MAX, UINT32_MAX, "", "");
                        }

                        /// Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture
                        // flags
                        TrackTextureAsset trackTextureAsset{track.trackTextureAssets.at(polygon.texture_id())};
                        std::vector<glm::vec2> temp_uvs{{1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}};
                        std::vector<glm::vec2> transformedUVs{trackTextureAsset.ScaleUVs(
                            temp_uvs, false, !polygon.invert(), polygon.rotate(), polygon.mirror_x(), polygon.mirror_y())};
                        xobj_uvs.insert(xobj_uvs.end(), transformedUVs.begin(), transformedUVs.end());

                        glm::vec3 const normal{
                            Utils::CalculateQuadNormal(extraObjectVerts[polygon.vertex[0]], extraObjectVerts[polygon.vertex[1]],
                                                       extraObjectVerts[polygon.vertex[2]], extraObjectVerts[polygon.vertex[3]])};

                        // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required
                        // per-vertex.
                        for (auto &quadToTriVertNumber : quadToTriVertNumbers) {
                            normals.emplace_back(normal);
                            vertexIndices.emplace_back(polygon.vertex[quadToTriVertNumber]);
                            textureIndices.emplace_back(polygon.texture_id());
                        }

                        accumulatedObjectFlags |= polygon.texflags;
                    }
                    glm::vec3 extraObjectCenter{objectHeader.pt * TRACK_SCALE_FACTOR};
                    auto extraObjectModel{TrackGeometry(extraObjectVerts, normals, xobj_uvs, textureIndices, vertexIndices,
                                                        extraObjectShadingData, extraObjectCenter)};
                    if (AnimKeyframes.empty()) {
                        globalObjects.emplace_back(objectIdx, EntityType::GLOBAL, extraObjectModel, accumulatedObjectFlags);
                    } else {
                        globalObjects.emplace_back(objectIdx, EntityType::GLOBAL, extraObjectModel, AnimKeyframes, animDelay,
                                                   accumulatedObjectFlags);
                    }
                }
            }
        }

        return {trackBlocks, globalObjects};
    }

    std::vector<TrackVRoad> Loader::_ParseVirtualRoad(FrdFile const &frdFile) {
        std::vector<TrackVRoad> virtualRoad;

        for (uint32_t vroadIdx{0}; vroadIdx < frdFile.numVRoad; ++vroadIdx) {
            VRoadBlock vroad{frdFile.vroadBlocks.at(vroadIdx)};

            // Transform NFS3/4 coords into ONFS 3d space
            glm::vec3 position{vroad.refPt * TRACK_SCALE_FACTOR};
            position.y += 0.2f;
            auto right{vroad.right / 128.f};
            auto forward{vroad.forward * TRACK_SCALE_FACTOR};
            auto normal{vroad.normal * TRACK_SCALE_FACTOR};

            glm::vec3 leftWall{(vroad.leftWall * TRACK_SCALE_FACTOR) * vroad.right};
            glm::vec3 rightWall{(vroad.rightWall * TRACK_SCALE_FACTOR) * vroad.right};

            virtualRoad.emplace_back(position, glm::vec3(0, 0, 0), normal, forward, right, leftWall, rightWall, vroad.unknown2[0]);
        }

        return virtualRoad;
    }
} // namespace LibOpenNFS::NFS4