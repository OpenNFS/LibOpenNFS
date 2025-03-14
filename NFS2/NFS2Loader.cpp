#include "NFS2Loader.h"

#include "Common/Logging.h"
#include "Common/TextureUtils.h"

#include <array>

namespace LibOpenNFS::NFS2 {
    /*template <typename Platform>
    Car Loader<Platform>::LoadCar(const std::string &carBasePath, NFSVersion nfsVersion) {
        std::filesystem::path p(carBasePath);
        std::string carName = p.filename().string();

        std::string geoPath = carBasePath + ".geo";
        std::string pshPath = carBasePath + ".psh";
        std::string qfsPath = carBasePath + ".qfs";
        std::string carOutPath;

        // For every file in here that's a BMP, load the data into a Texture object. This lets us easily access textures
    by an ID. std::map<uint32_t, Texture> carTextures; std::map<std::string, uint32_t> remappedTextureIds; uint32_t
    remappedTextureID = 0;

        // TODO: Refactor all of this to nexgen style
        switch (nfsVersion) {
        case NFSVersion::NFS_3_PS1:
        case NFSVersion::NFS_2_PS1:
            carOutPath = LibOpenNFS::CAR_PATH + get_string(nfsVersion) + "/" + carName + "/";
            // ImageLoader::ExtractPSH(pshPath, carOutPath);
            break;
        case NFSVersion::NFS_2:
        case NFSVersion::NFS_2_SE:
            carOutPath = CAR_PATH + get_string(nfsVersion) + "/" + carName + "/";
            // ImageLoader::ExtractQFS(qfsPath, carOutPath);
            break;
        default:
            ASSERT(false, "I shouldn't be loading this version (" << get_string(nfsVersion) << ") and you know it");
        }

        for (std::filesystem::directory_iterator itr(carOutPath); itr != std::filesystem::directory_iterator(); ++itr) {
            if (itr->path().filename().string().find("BMP") != std::string::npos &&
    itr->path().filename().string().find("-a") == std::string::npos) {
                // Map texture names, strings, into numbers so I can use them for indexes into the eventual Texture
    Array remappedTextureIds[itr->path().filename().replace_extension("").string()] = remappedTextureID++; switch
    (nfsVersion) { case NFSVersion::NFS_3_PS1: case NFSVersion::NFS_2_PS1: GLubyte *data; GLsizei width; GLsizei height;
                    ASSERT(ImageLoader::LoadBmpCustomAlpha(itr->path().string().c_str(), &data, &width, &height, 0u),
                           "Texture " << itr->path().string() << " did not load succesfully!");
                    // carTextures[remappedTextureIds[itr->path().filename().replace_extension("").string()]] =
                    //   Texture(nfsVersion, remappedTextureIds[itr->path().filename().replace_extension("").string()],
    data, static_cast<unsigned int>(width),
                    //           static_cast<unsigned int>(height), nullptr);
                    break;
                case NFSVersion::NFS_2:
                case NFSVersion::NFS_2_SE:
                    bmpread_t bmpAttr; // This will leak.
                    ASSERT(bmpread(itr->path().string().c_str(), BMPREAD_ANY_SIZE | BMPREAD_ALPHA, &bmpAttr), "Texture "
    << itr->path().string() << " did not load succesfully!");
                    // carTextures[remappedTextureIds[itr->path().filename().replace_extension("").string()]] =
                    //  Texture(nfsVersion, remappedTextureIds[itr->path().filename().replace_extension("").string()],
    bmpAttr.data, static_cast<unsigned int>(bmpAttr.width),
                    //          static_cast<unsigned int>(bmpAttr.height), nullptr);
                    break;
                default:
                    ASSERT(false, "I shouldn't be loading this version (" << get_string(nfsVersion) << ")");
                }
            }
        }

        GeoFile<Platform> geoFile;
        ASSERT(GeoFile<Platform>::Load(geoPath, geoFile), "Could not load GEO file: " << geoPath);

        // This must run before geometry load, as it will affect UV's
        GLint textureArrayID  = GLTexture::MakeTextureArray(carTextures, false);
        Car::MetaData carData = _ParseGEOModels(geoFile);
        // carData.meshes = LoadGEO(geo_path.str(), car_textures, remapped_texture_ids);
        return std::make_shared<Car>(carData, NFSVersion::NFS_2, carName, textureArrayID);
    }*/

    template <> Track Loader<PC>::LoadTrack(NFSVersion nfsVersion, std::string const &trackBasePath, std::string const &trackOutPath) {
        LogInfo("Loading Track located at %s", trackBasePath.c_str());
        std::filesystem::path p(trackBasePath);
        Track track(nfsVersion, p.filename().string(), trackBasePath);

        std::string trkPath, colPath, canPath;
        trkPath = trackBasePath + ".trk";
        colPath = trackBasePath + ".col";

        switch (nfsVersion) {
        case NFSVersion::NFS_2:
        case NFSVersion::NFS_2_SE:
            canPath = p.parent_path().parent_path().string() + "/data/pc/" + track.name + "00.can";
            break;
        case NFSVersion::NFS_2_PS1:
            canPath = trackBasePath + "00.can";
            break;
        default:
            ASSERT(false, "Attempting to load unknown NFS version with NFS2 PC Track parser");
        }

        TrkFile<PC> trkFile;
        ColFile<PC> colFile;
        Shared::CanFile canFile;

        ASSERT(Shared::CanFile::Load(canPath, canFile),
               "Could not load CAN file (camera animation): " << canPath); // Load camera intro/outro animation data
        ASSERT(TrkFile<PC>::Load(trkPath, trkFile, track.nfsVersion),
               "Could not load TRK file: " << trkPath); // Load TRK file to get track block specific data
        ASSERT(ColFile<PC>::Load(colPath, colFile, track.nfsVersion),
               "Could not load COL file: " << colPath); // Load Catalogue file to get global (non block specific) data

        track.nBlocks = trkFile.nBlocks;
        track.cameraAnimation = canFile.animPoints;
        track.trackTextureAssets = _ParseTextures(track, trackOutPath);
        track.trackBlocks = _ParseTRKModels(trkFile, colFile, track);
        track.globalObjects = _ParseCOLModels(colFile, track);
        track.virtualRoad = _ParseVirtualRoad(colFile);

        LogInfo("Track loaded successfully");

        return track;
    }

    template <> Track Loader<PS1>::LoadTrack(NFSVersion nfsVersion, std::string const &trackBasePath, std::string const &trackOutPath) {
        LogInfo("Loading Track located at %s", trackBasePath.c_str());
        std::filesystem::path p(trackBasePath);
        Track track(nfsVersion, p.filename().string(), trackBasePath);

        std::string trkPath, colPath;

        trkPath = trackBasePath + ".trk";
        colPath = trackBasePath + ".col";
        colPath.replace(colPath.find("zz"), 2, "");

        TrkFile<PS1> trkFile;
        ColFile<PS1> colFile;

        ASSERT(TrkFile<PS1>::Load(trkPath, trkFile, nfsVersion),
               "Could not load TRK file: " << trkPath); // Load TRK file to get track block specific data
        ASSERT(ColFile<PS1>::Load(colPath, colFile, nfsVersion),
               "Could not load COL file: " << colPath); // Load Catalogue file to get global (non block specific) data

        // track.textureArrayID = Texture::MakeTextureArray(track->textureMap, false);
        track.nBlocks = trkFile.nBlocks;
        track.trackTextureAssets = _ParseTextures(track, trackOutPath);
        track.trackBlocks = _ParseTRKModels(trkFile, colFile, track);
        track.globalObjects = _ParseCOLModels(colFile, track);
        track.virtualRoad = _ParseVirtualRoad(colFile);

        LogInfo("Track loaded successfully");

        return track;
    }

    template <typename Platform> Car::MetaData Loader<Platform>::_ParseGEOModels(GeoFile<Platform> const &geoFile) {
        ASSERT(false, "Unimplemented");
        return Car::MetaData();
    }

    template <typename Platform>
    std::map<uint32_t, TrackTextureAsset> Loader<Platform>::_ParseTextures(Track const &track, std::string const &trackOutPath) {
        using namespace std::filesystem;

        // TODO: Hack :( OpenNFS needs to scale the UVs by the proportion of the textures size to the max sized texture on the track,
        // due to the usage of a texture array in the renderer. NFS2 doesn't encode the size of the texture inside the FRD file, hence
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

    // One might question why a TRK parsing function requires the COL file too. Simples, we need XBID 2 for Texture
    // remapping during ONFS texgen.
    template <typename Platform>
    std::vector<LibOpenNFS::TrackBlock> Loader<Platform>::_ParseTRKModels(TrkFile<Platform> const &trkFile,
                                                                          ColFile<Platform> &colFile,
                                                                          Track const &track) {
        LogInfo("Parsing TRK file into ONFS GL structures");
        std::vector<LibOpenNFS::TrackBlock> trackBlocks;

        // Pull out a shorter reference to the texture table
        auto polyToQfsTexTable = colFile.GetExtraObjectBlock(ExtraBlockID::TEXTURE_BLOCK_ID).polyToQfsTexTable;

        // Parse out TRKBlock data
        for (auto const &superBlock : trkFile.superBlocks) {
            for (auto rawTrackBlock : superBlock.trackBlocks) {
                // Get position all vertices need to be relative to
                glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-glm::pi<float>() / 2, 0, 0)));
                glm::vec3 rawTrackBlockCenter =
                    orientation * (Utils::PointToVec(trkFile.blockReferenceCoords[rawTrackBlock.serialNum]) / NFS2_SCALE_FACTOR);
                std::vector<uint32_t> trackBlockNeighbourIds;

                // Convert the neighbor int16_t's to uint32_t for OFNS trackblock representation
                if (rawTrackBlock.IsBlockPresent(ExtraBlockID::NEIGHBOUR_BLOCK_ID)) {
                    // if the numbers go beyond the track length they start back at 0, and if they drop below 0 they
                    // start back at the track length - 1
                    for (auto &trackBlockNeighbourRaw :
                         rawTrackBlock.GetExtraObjectBlock(ExtraBlockID::NEIGHBOUR_BLOCK_ID).blockNeighbours) {
                        trackBlockNeighbourIds.push_back(trackBlockNeighbourRaw % trkFile.nBlocks);
                    }
                }

                // Count the number of virtual road positions for this trackblock
                uint32_t nVroadPositions = 0;
                uint32_t vroadStartIndex = 0;
                auto collisionBlock = colFile.GetExtraObjectBlock(ExtraBlockID::COLLISION_BLOCK_ID);
                for (uint32_t vroadIdx = 0; vroadIdx < collisionBlock.nCollisionData; ++vroadIdx) {
                    auto vroadEntry = collisionBlock.collisionData[vroadIdx];
                    if (vroadEntry.blockNumber == rawTrackBlock.serialNum) {
                        if (nVroadPositions == 0) {
                            vroadStartIndex = vroadIdx;
                        }
                        ++nVroadPositions;
                    }
                }

                // Build the base OpenNFS trackblock, to hold all of the geometry and virtual road data, lights, sounds
                // etc. for this portion of track
                LibOpenNFS::TrackBlock trackBlock(rawTrackBlock.serialNum, rawTrackBlockCenter, vroadStartIndex, nVroadPositions,
                                                  trackBlockNeighbourIds);

                // Collate all available Structure References, 3 different ID types can store this information, check
                // them all
                std::vector<StructureRefBlock> structureReferences;
                for (auto &structRefBlockId : {ExtraBlockID::STRUCTURE_REF_BLOCK_A_ID, ExtraBlockID::STRUCTURE_REF_BLOCK_B_ID,
                                               ExtraBlockID::STRUCTURE_REF_BLOCK_C_ID}) {
                    if (rawTrackBlock.IsBlockPresent(structRefBlockId)) {
                        auto structureRefBlock = rawTrackBlock.GetExtraObjectBlock(structRefBlockId);
                        structureReferences.insert(structureReferences.end(), structureRefBlock.structureReferences.begin(),
                                                   structureRefBlock.structureReferences.end());
                    }
                }

                // Pull out structures from trackblock if present
                if (rawTrackBlock.IsBlockPresent(ExtraBlockID::STRUCTURE_BLOCK_ID)) {
                    // Check whether there are enough struct references for how many strucutres there are for this
                    // trackblock
                    if (rawTrackBlock.GetExtraObjectBlock(ExtraBlockID::STRUCTURE_BLOCK_ID).nStructures != structureReferences.size()) {
                        LogWarning("Trk block %d is missing %d structure locations!", (int)rawTrackBlock.serialNum,
                                   rawTrackBlock.GetExtraObjectBlock(ExtraBlockID::STRUCTURE_BLOCK_ID).nStructures -
                                       structureReferences.size());
                    }

                    // Shorter reference to structures for trackblock
                    auto structures = rawTrackBlock.GetExtraObjectBlock(ExtraBlockID::STRUCTURE_BLOCK_ID).structures;

                    // Structures
                    for (uint32_t structureIdx = 0;
                         structureIdx < rawTrackBlock.GetExtraObjectBlock(ExtraBlockID::STRUCTURE_BLOCK_ID).nStructures; ++structureIdx) {
                        // Mesh Data
                        std::vector<uint32_t> structureVertexIndices;
                        std::vector<glm::vec2> structureUVs;
                        std::vector<uint32_t> structureTextureIndices;
                        std::vector<glm::vec3> structureVertices;
                        std::vector<glm::vec4> structureShadingData;
                        std::vector<glm::vec3> structureNormals;

                        // Find the structure reference that matches this structure, else use block default
                        VERT_HIGHP structureReferenceCoordinates = trkFile.blockReferenceCoords[rawTrackBlock.serialNum];
                        bool refCoordsFound = false;

                        for (auto &structureReference : structureReferences) {
                            // Only check fixed type structure references
                            if (structureReference.structureRef == structureIdx) {
                                if (structureReference.recType == 1 || structureReference.recType == 4) {
                                    structureReferenceCoordinates = structureReference.refCoordinates;
                                    refCoordsFound = true;
                                    break;
                                }
                                if (structureReference.recType == 3) {
                                    // For now, if animated, use position 0 of animation sequence
                                    structureReferenceCoordinates = structureReference.animationData[0].position;
                                    refCoordsFound = true;
                                    break;
                                }
                            }
                        }
                        if (!refCoordsFound) {
                            LogWarning("Couldn't find a reference coordinate for Structure %d in TB %d", structureIdx,
                                       rawTrackBlock.serialNum);
                        }
                        for (uint16_t vertIdx = 0; vertIdx < structures[structureIdx].nVerts; ++vertIdx) {
                            structureVertices.emplace_back(
                                orientation *
                                ((256.f * Utils::PointToVec(structures[structureIdx].vertexTable[vertIdx])) / NFS2_SCALE_FACTOR));
                            structureShadingData.emplace_back(1.0, 1.0f, 1.0f, 1.0f);
                        }
                        for (uint32_t polyIdx = 0; polyIdx < structures[structureIdx].nPoly; ++polyIdx) {
                            // Remap the COL TextureID's using the COL texture block (XBID2)
                            TEXTURE_BLOCK polygonTexture = polyToQfsTexTable[structures[structureIdx].polygonTable[polyIdx].texture];
                            TrackTextureAsset textureAsset = track.trackTextureAssets.at(polygonTexture.texNumber);
                            // Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture flags
                            std::vector<glm::vec2> uvs{{1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}};
                            std::vector<glm::vec2> transformedUVs = textureAsset.ScaleUVs(uvs, false, std::is_same_v<Platform, PS1>, 0);
                            structureUVs.insert(structureUVs.end(), transformedUVs.begin(), transformedUVs.end());

                            // Calculate the normal, as no provided data
                            glm::vec3 normal =
                                Utils::CalculateQuadNormal(structureVertices[structures[structureIdx].polygonTable[polyIdx].vertex[0]],
                                                           structureVertices[structures[structureIdx].polygonTable[polyIdx].vertex[1]],
                                                           structureVertices[structures[structureIdx].polygonTable[polyIdx].vertex[2]],
                                                           structureVertices[structures[structureIdx].polygonTable[polyIdx].vertex[3]]);

                            // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required
                            // per-vertex.
                            for (auto &quadToTriVertNumber : quadToTriVertNumbers) {
                                structureNormals.emplace_back(normal);
                                structureVertexIndices.emplace_back(
                                    structures[structureIdx].polygonTable[polyIdx].vertex[quadToTriVertNumber]);
                                structureTextureIndices.emplace_back(polygonTexture.texNumber);
                            }
                        }

                        auto structureModel = TrackGeometry(
                            structureVertices, structureNormals, structureUVs, structureTextureIndices, structureVertexIndices,
                            structureShadingData, orientation * (Utils::PointToVec(structureReferenceCoordinates) / NFS2_SCALE_FACTOR));
                        trackBlock.objects.emplace_back(rawTrackBlock.serialNum + structureIdx, EntityType::OBJ_POLY, structureModel, 0);
                    }
                }

                // Track Mesh Data
                std::vector<uint32_t> trackBlockVertexIndices;
                std::vector<glm::vec2> trackBlockUVs;
                std::vector<uint32_t> trackBlockTextureIndices;
                std::vector<glm::vec3> trackBlockVertices;
                std::vector<glm::vec4> trackBlockShadingData;
                std::vector<glm::vec3> trackBlockNormals;

                // Base Track Geometry
                VERT_HIGHP blockRefCoord = {};

                for (int32_t vertIdx = 0; vertIdx < rawTrackBlock.nStickToNextVerts + rawTrackBlock.nHighResVert; vertIdx++) {
                    if (vertIdx < rawTrackBlock.nStickToNextVerts) {
                        // If in last block go get ref coord of first block, else get ref of next block
                        blockRefCoord = (rawTrackBlock.serialNum == track.nBlocks - 1)
                                            ? trkFile.blockReferenceCoords[0]
                                            : trkFile.blockReferenceCoords[rawTrackBlock.serialNum + 1];
                    } else {
                        blockRefCoord = trkFile.blockReferenceCoords[rawTrackBlock.serialNum];
                    }

                    trackBlockVertices.emplace_back(orientation * (((Utils::PointToVec(blockRefCoord) +
                                                                     (256.f * Utils::PointToVec(rawTrackBlock.vertexTable[vertIdx]))) /
                                                                    NFS2_SCALE_FACTOR)));
                    trackBlockShadingData.emplace_back(1.f, 1.f, 1.f, 1.f);
                }
                for (int32_t polyIdx = (rawTrackBlock.nLowResPoly + rawTrackBlock.nMedResPoly);
                     polyIdx < (rawTrackBlock.nLowResPoly + rawTrackBlock.nMedResPoly + rawTrackBlock.nHighResPoly); ++polyIdx) {
                    // Remap the COL TextureID's using the COL texture block (XBID2)
                    TEXTURE_BLOCK polygonTexture = polyToQfsTexTable[rawTrackBlock.polygonTable[polyIdx].texture];
                    TrackTextureAsset textureAsset = track.trackTextureAssets.at(polygonTexture.texNumber);
                    // Convert the UV's into ONFS space, to enable tiling/mirroring etc based on NFS texture flags
                    std::vector<glm::vec2> uvs{{1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}};
                    std::vector<glm::vec2> transformedUVs =
                        textureAsset.ScaleUVs(uvs, false, std::is_same_v<Platform, PS1>, (polygonTexture.alignmentData >> 11) & 3);
                    trackBlockUVs.insert(trackBlockUVs.end(), transformedUVs.begin(), transformedUVs.end());
                    // Calculate the normal, as no provided data
                    glm::vec3 normal = Utils::CalculateQuadNormal(trackBlockVertices[rawTrackBlock.polygonTable[polyIdx].vertex[0]],
                                                                  trackBlockVertices[rawTrackBlock.polygonTable[polyIdx].vertex[1]],
                                                                  trackBlockVertices[rawTrackBlock.polygonTable[polyIdx].vertex[2]],
                                                                  trackBlockVertices[rawTrackBlock.polygonTable[polyIdx].vertex[3]]);

                    // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                    for (auto &quadToTriVertNumber : quadToTriVertNumbers) {
                        trackBlockNormals.emplace_back(normal);
                        trackBlockVertexIndices.emplace_back(rawTrackBlock.polygonTable[polyIdx].vertex[quadToTriVertNumber]);
                        trackBlockTextureIndices.emplace_back(polygonTexture.texNumber);
                    }
                }

                TrackGeometry trackBlockModel(trackBlockVertices, trackBlockNormals, trackBlockUVs, trackBlockTextureIndices,
                                              trackBlockVertexIndices, trackBlockShadingData, glm::vec3());
                trackBlock.track.emplace_back(rawTrackBlock.serialNum, EntityType::ROAD, trackBlockModel, 0);

                // Add the parsed ONFS trackblock to the list of trackblocks
                trackBlocks.push_back(trackBlock);
            }
        }
        return trackBlocks;
    }

    template <typename Platform> std::vector<TrackVRoad> Loader<Platform>::_ParseVirtualRoad(ColFile<Platform> &colFile) {
        std::vector<TrackVRoad> virtualRoad;

        glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-glm::pi<float>() / 2, 0, 0)));

        if (!colFile.IsBlockPresent(ExtraBlockID::COLLISION_BLOCK_ID)) {
            LogWarning("Col file is missing virtual road data");
            return virtualRoad;
        }

        for (auto &vroadEntry : colFile.GetExtraObjectBlock(ExtraBlockID::COLLISION_BLOCK_ID).collisionData) {
            // Transform NFS2 coords into ONFS 3d space
            glm::vec3 vroadCenter = orientation * (Utils::PointToVec(vroadEntry.trackPosition) / NFS2_SCALE_FACTOR);
            vroadCenter.y += 0.2f;

            // Get VROAD forward and normal vectors, fake a right vector
            glm::vec3 right = orientation * glm::vec3(vroadEntry.rightVec[0], vroadEntry.rightVec[2], vroadEntry.rightVec[1]);
            glm::vec3 forward = orientation * glm::vec3(vroadEntry.fwdVec[0], vroadEntry.fwdVec[2], vroadEntry.fwdVec[1]) / 256.f;
            glm::vec3 normal = orientation * glm::vec3(vroadEntry.vertVec[0], vroadEntry.vertVec[2], vroadEntry.vertVec[1]) / 256.f;

            glm::vec3 leftWall = (vroadEntry.leftBorder / NFS2_SCALE_FACTOR) * right * 2.f;
            glm::vec3 rightWall = (vroadEntry.rightBorder / NFS2_SCALE_FACTOR) * right * 2.f;
            glm::vec3 lateralRespawn = ((vroadEntry.postCrashPosition) / NFS2_SCALE_FACTOR) * right; // TODO: This is incorrect

            virtualRoad.emplace_back(vroadCenter, glm::vec3(), normal, forward, right, leftWall, rightWall, vroadEntry.unknown2);
        }

        return virtualRoad;
    }

    template class Loader<PC>;

    template <typename Platform>
    std::vector<TrackEntity> Loader<Platform>::_ParseCOLModels(ColFile<Platform> &colFile, Track const &track) {
        LogInfo("Parsing COL file into ONFS GL structures");
        std::vector<TrackEntity> colEntities;

        // All Vertices are stored so that the model is rotated 90 degs on X. Remove this at Vert load time.
        glm::quat orientation = glm::normalize(glm::quat(glm::vec3(-glm::pi<float>() / 2, 0, 0)));

        // Shorter reference to structures and texture table
        auto structures = colFile.GetExtraObjectBlock(ExtraBlockID::STRUCTURE_BLOCK_ID).structures;
        auto polyToQfsTexTable = colFile.GetExtraObjectBlock(ExtraBlockID::TEXTURE_BLOCK_ID).polyToQfsTexTable;

        // Parse out COL data
        for (uint32_t structureIdx = 0; structureIdx < colFile.GetExtraObjectBlock(ExtraBlockID::STRUCTURE_BLOCK_ID).nStructures;
             ++structureIdx) {
            std::vector<uint32_t> globalStructureVertexIndices;
            std::vector<glm::vec2> globalStructureUVs;
            std::vector<uint32_t> globalStructureTextureIndices;
            std::vector<glm::vec3> globalStructureVertices;
            std::vector<glm::vec4> globalStructureShadingData;
            std::vector<glm::vec3> globalStructureNormals;

            VERT_HIGHP structureReferenceCoordinates = {};
            bool refCoordsFound = false;
            // Find the structure reference that matches this structure
            for (auto &structure : colFile.GetExtraObjectBlock(ExtraBlockID::STRUCTURE_REF_BLOCK_A_ID).structureReferences) {
                // Only check fixed type structure references
                if (structure.structureRef == structureIdx) {
                    if (structure.recType == 1 || structure.recType == 4) {
                        structureReferenceCoordinates = structure.refCoordinates;
                        refCoordsFound = true;
                        break;
                    }
                    if (structure.recType == 3) {
                        // For now, if animated, use position 0 of animation sequence
                        structureReferenceCoordinates = structure.animationData[0].position;
                        refCoordsFound = true;
                        break;
                    }
                }
            }
            if (!refCoordsFound) {
                LogWarning("Couldn't find a reference coordinate for Structure %d in COL file", structureIdx);
            }
            for (uint16_t vertIdx = 0; vertIdx < structures[structureIdx].nVerts; ++vertIdx) {
                globalStructureVertices.emplace_back(
                    orientation * ((256.f * Utils::PointToVec(structures[structureIdx].vertexTable[vertIdx])) / NFS2_SCALE_FACTOR));
                globalStructureShadingData.emplace_back(1.0, 1.0f, 1.0f, 1.0f);
            }

            for (uint32_t polyIdx = 0; polyIdx < structures[structureIdx].nPoly; ++polyIdx) {
                // Remap the COL TextureID's using the COL texture block (XBID2)
                TEXTURE_BLOCK polygonTexture = polyToQfsTexTable[structures[structureIdx].polygonTable[polyIdx].texture];
                TrackTextureAsset textureAsset = track.trackTextureAssets.at(polygonTexture.texNumber);
                // Calculate the normal, as no provided data
                glm::vec3 normal =
                    Utils::CalculateQuadNormal(globalStructureVertices[structures[structureIdx].polygonTable[polyIdx].vertex[0]],
                                               globalStructureVertices[structures[structureIdx].polygonTable[polyIdx].vertex[1]],
                                               globalStructureVertices[structures[structureIdx].polygonTable[polyIdx].vertex[2]],
                                               globalStructureVertices[structures[structureIdx].polygonTable[polyIdx].vertex[3]]);

                // TODO: Use textures alignment data to modify these UV's
                globalStructureUVs.emplace_back(1.0f * textureAsset.maxU, 1.0f * textureAsset.maxV);
                globalStructureUVs.emplace_back(0.0f * textureAsset.maxU, 1.0f * textureAsset.maxV);
                globalStructureUVs.emplace_back(0.0f * textureAsset.maxU, 0.0f * textureAsset.maxV);
                globalStructureUVs.emplace_back(1.0f * textureAsset.maxU, 1.0f * textureAsset.maxV);
                globalStructureUVs.emplace_back(0.0f * textureAsset.maxU, 0.0f * textureAsset.maxV);
                globalStructureUVs.emplace_back(1.0f * textureAsset.maxU, 0.0f * textureAsset.maxV);

                // Two triangles per raw quad, hence 6 vertices. Normal data and texture index required per-vertex.
                for (auto &quadToTriVertNumber : quadToTriVertNumbers) {
                    globalStructureNormals.push_back(normal);
                    globalStructureVertexIndices.push_back(structures[structureIdx].polygonTable[polyIdx].vertex[quadToTriVertNumber]);
                    globalStructureTextureIndices.push_back(polygonTexture.texNumber);
                }
            }

            glm::vec3 position = orientation * (Utils::PointToVec(structureReferenceCoordinates) / NFS2_SCALE_FACTOR);
            TrackGeometry globalStructureModel(globalStructureVertices, globalStructureNormals, globalStructureUVs,
                                               globalStructureTextureIndices, globalStructureVertexIndices, globalStructureShadingData,
                                               position);
            colEntities.emplace_back(structureIdx, EntityType::GLOBAL, globalStructureModel, 0);
        }

        return colEntities;
    }
    template class Loader<PS1>;

} // namespace LibOpenNFS::NFS2
