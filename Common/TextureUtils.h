#pragma once

#include <filesystem>
#include <string>

#include "NFSVersion.h"
#include "Shared/FSH/FshFile.h"
#include "Utils.h"

namespace LibOpenNFS {
    class TextureUtils {
      public:
        static uint32_t abgr1555ToARGB8888(uint16_t abgr_1555);
        static glm::vec4 HSLToRGB(glm::vec4 hsl);
        static glm::vec3 ParseRGBString(std::string const &rgb_string);
        // Break Packed uint32_t RGBA per vertex colour data for baked lighting of RGB into 4 normalised floats and store into vec4
        static glm::vec4 ShadingDataToVec4(uint32_t packed_rgba);
        static bool ExtractQFS(std::string const &qfs_input, std::string const &output_dir, bool skipMirrored = false);
        static bool ExtractTrackTextures(std::string const &trackPath, ::std::string const &trackName, NFSVersion nfsVer,
                                         std::string const &outPath);
        static std::tuple<uint32_t, uint32_t> GetBitmapDimensions(std::string const &texturePath);
    };

} // namespace LibOpenNFS