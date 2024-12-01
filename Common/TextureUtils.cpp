#include "TextureUtils.h"

#include <sstream>

#include "Common/Logging.h"

namespace LibOpenNFS {
    uint32_t TextureUtils::abgr1555ToARGB8888(uint16_t const abgr_1555) {
        auto const red{static_cast<uint8_t>(round((abgr_1555 & 0x1F) / 31.0F * 255.0F))};
        auto const green{static_cast<uint8_t>(round(((abgr_1555 & 0x3E0) >> 5) / 31.0F * 255.0F))};
        auto const blue{static_cast<uint8_t>(round(((abgr_1555 & 0x7C00) >> 10) / 31.0F * 255.0F))};

        uint32_t alpha = 255;
        if (((abgr_1555 & 0x8000) == 0 ? 1 : 0) == ((red == 0) && (green == 0) && (blue == 0) ? 1 : 0)) {
            alpha = 0;
        }

        return alpha << 24 | red << 16 | green << 8 | blue;
    }

    // Convert hsv floats ([0-1],[0-1],[0-1]) to rgb floats ([0-1],[0-1],[0-1]), from Foley & van Dam p593
    // also http://en.wikipedia.org/wiki/HSL_and_HSV
    glm::vec3 TextureUtils::HSLToRGB(glm::vec4 hsl) {
        float h = hsl.x / 255.f;
        float s = hsl.y / 255.f;
        float v = hsl.z / 255.f;
        glm::vec3 rgb;

        if (s == 0.0f) {
            // gray
            rgb.r = rgb.g = rgb.b = v;
            return rgb;
        }

        h = fmodf(h, 1.0f) / (60.0f / 360.0f);
        int i = (int)h;
        float f = h - (float)i;
        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t = v * (1.0f - s * (1.0f - f));

        switch (i) {
        case 0:
            rgb.r = v;
            rgb.g = t;
            rgb.b = p;
            break;
        case 1:
            rgb.r = q;
            rgb.g = v;
            rgb.b = p;
            break;
        case 2:
            rgb.r = p;
            rgb.g = v;
            rgb.b = t;
            break;
        case 3:
            rgb.r = p;
            rgb.g = q;
            rgb.b = v;
            break;
        case 4:
            rgb.r = t;
            rgb.g = p;
            rgb.b = v;
            break;
        case 5:
        default:
            rgb.r = v;
            rgb.g = p;
            rgb.b = q;
            break;
        }

        return rgb;
    }

    glm::vec3 TextureUtils::ParseRGBString(std::string const &rgb_string) {
        std::stringstream tempComponent;
        uint8_t commaCount = 0;
        glm::vec3 rgbValue;

        for (int char_Idx = 0; char_Idx < rgb_string.length(); ++char_Idx) {
            if (rgb_string[char_Idx] == ',') {
                switch (commaCount) {
                case 0:
                    rgbValue.x = (float)stoi(tempComponent.str());
                    break;
                case 1:
                    rgbValue.y = (float)stoi(tempComponent.str());
                    break;
                case 2:
                    rgbValue.z = (float)stoi(tempComponent.str());
                    break;
                default:
                    ASSERT(false, "Attempted to parse 4 component RGB");
                }
                tempComponent.str("");
                if (++commaCount >= 3) {
                    break;
                }
            } else {
                tempComponent << rgb_string[char_Idx];
            }
        }

        return rgbValue;
    }

    glm::vec4 TextureUtils::ShadingDataToVec4(uint32_t packed_rgba) {
        return {((packed_rgba >> 16) & 0xFF) / 255.0f, ((packed_rgba >> 8) & 0xFF) / 255.0f,
                (packed_rgba & 0xFF) / 255.0f, ((packed_rgba >> 24) & 0xFF) / 255.0f};
    }

    bool TextureUtils::ExtractQFS(std::string const &qfs_input, std::string const &output_dir) {
        LogInfo("Extracting QFS file: %s to %s", qfs_input.c_str(), output_dir.c_str());
        if (std::filesystem::exists(output_dir)) {
            LogInfo("Textures already exist at %s, nothing to extract", output_dir.c_str());
            return true;
        }

        std::filesystem::create_directories(output_dir);

        // Fshtool changes the current working directory, save and restore
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));

        char *args[3] = {const_cast<char *>(""), strdup(qfs_input.c_str()), strdup(output_dir.c_str())};
        int returnCode = (fsh_main(3, args) == 1);

        chdir(cwd);

        return returnCode;
    }

    bool TextureUtils::ExtractTrackTextures(std::string const &trackPath,
                                            ::std::string const &trackName,
                                            NFSVersion nfsVer,
                                            std::string const &outPath) {
        if (std::filesystem::exists(outPath)) {
            return true;
        }
        std::filesystem::create_directories(outPath);

        std::stringstream nfsTexArchivePath;
        std::string fullTrackPath = trackPath + "/" + trackName;

        switch (nfsVer) {
        case NFSVersion::NFS_2:
            nfsTexArchivePath << trackPath << "0.qfs";
            break;
        case NFSVersion::NFS_2_SE:
            nfsTexArchivePath << trackPath << "0m.qfs";
            break;
        case NFSVersion::NFS_2_PS1:
            nfsTexArchivePath << trackPath << "0.psh";
            break;
        case NFSVersion::NFS_3:
            nfsTexArchivePath << fullTrackPath << "0.qfs";
            break;
        case NFSVersion::NFS_3_PS1: {
            std::string pshPath = trackPath;
            pshPath.replace(pshPath.find("zz"), 2, "");
            nfsTexArchivePath << pshPath << "0.psh";
        } break;
        case NFSVersion::NFS_4:
            nfsTexArchivePath << trackPath << "/tr0.qfs";
            break;
        case NFSVersion::UNKNOWN:
        default:
            ASSERT(false, "Trying to extract track textures from unknown NFS version");
            break;
        }

        LogInfo("Extracting track textures");
        std::string onfsTrackAssetTextureDir = outPath + "/textures/";

        switch (nfsVer) {
        case NFSVersion::NFS_2_PS1:
        case NFSVersion::NFS_3_PS1:
            return true;
            // return ImageLoader::ExtractPSH(nfsTexArchivePath.str(), onfsTrackAssetTextureDir);
        case NFSVersion::NFS_3: {
            std::stringstream nfsSkyTexArchivePath;
            nfsSkyTexArchivePath << fullTrackPath.substr(0, fullTrackPath.find_last_of('/')) << "/sky.fsh";
            if (std::filesystem::exists(nfsSkyTexArchivePath.str())) {
                std::string onfsTrackAssetSkyTexDir = outPath + "/sky_textures/";
                ASSERT(ExtractQFS(nfsSkyTexArchivePath.str(), onfsTrackAssetSkyTexDir),
                       "Unable to extract sky textures from " << nfsSkyTexArchivePath.str());
            }
        } break;
        default:
            break;
        }

        return ExtractQFS(nfsTexArchivePath.str(), onfsTrackAssetTextureDir);
    }
} // namespace LibOpenNFS