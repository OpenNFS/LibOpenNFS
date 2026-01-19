#pragma once

#include "FshTypes.h"
#include "QfsCompression.h"
#include <algorithm>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace LibOpenNFS::Shared {

    /**
     * Represents a texture/bitmap extracted from an FSH archive.
     * Provides methods to access raw pixel data and convert to various formats.
     */
    class FshTexture {
      public:
        FshTexture() = default;

        FshTexture(std::string name, uint16_t const width, uint16_t const height, PixelFormat const format)
            : m_name(std::move(name)), m_width(width), m_height(height), m_format(format) {
        }

        // Accessors
        std::string const &Name() const {
            return m_name;
        }
        uint16_t Width() const {
            return m_width;
        }
        uint16_t Height() const {
            return m_height;
        }
        PixelFormat Format() const {
            return m_format;
        }

        bool HasPalette() const {
            return m_format == PixelFormat::Indexed8Bit ||  m_format == PixelFormat::Indexed8BitPSH ||m_format == PixelFormat::Indexed4Bit;
        }
        bool HasAlpha() const {
            return HasAlphaChannel(m_format) || m_hasAlphaAttachment;
        }
        bool IsCompressed() const {
            return IsCompressedFormat(m_format);
        }

        std::vector<uint8_t> const &RawData() const {
            return m_rawData;
        }
        std::vector<uint8_t> &RawData() {
            return m_rawData;
        }

        Palette const &GetPalette() const {
            return m_palette;
        }
        Palette &GetPalette() {
            return m_palette;
        }

        std::vector<uint8_t> const &AlphaData() const {
            return m_alphaData;
        }
        std::vector<uint8_t> &AlphaData() {
            return m_alphaData;
        }

        void SetHasAlphaAttachment(bool const value) {
            m_hasAlphaAttachment = value;
        }

        /**
         * Convert texture to 32-bit ARGB pixel data
         * @return Vector of ARGB pixels (row-major, bottom-to-top for BMP compatibility)
         */
        std::vector<uint32_t> ToARGB32() const;

        /**
         * Convert texture to RGBA byte array (suitable for OpenGL)
         * @return Vector of RGBA bytes (4 bytes per pixel: R, G, B, A)
         */
        std::vector<uint8_t> ToRGBA() const;

        /**
         * Export texture to BMP file
         * @param filepath Output file path
         * @param includeAlpha If true, export as 32-bit BGRA BMP with alpha channel embedded
         * @return true on success
         */
        bool ExportToBmp(std::string const &filepath, bool includeAlpha = false) const;

        /**
         * Export alpha channel to separate BMP file (8-bit grayscale)
         * @param filepath Output file path
         * @return true on success
         */
        bool ExportAlphaToBmp(std::string const &filepath) const;

      private:
        std::string m_name;
        uint16_t m_width = 0;
        uint16_t m_height = 0;
        PixelFormat m_format = PixelFormat::Unknown;
        std::vector<uint8_t> m_rawData;
        Palette m_palette;
        std::vector<uint8_t> m_alphaData;
        bool m_hasAlphaAttachment = false;

        // Conversion methods
        void ConvertIndexed4ToARGB32(std::vector<uint32_t> &output) const;
        void ConvertIndexed8ToARGB32(std::vector<uint32_t> &output) const;
        void ConvertARGB32(std::vector<uint32_t> &output) const;
        void ConvertRGB24ToARGB32(std::vector<uint32_t> &output) const;
        void ConvertARGB16_1555ToARGB32(std::vector<uint32_t> &output) const;
        void ConvertABGR16_1555ToARGB32(std::vector<uint32_t> &output) const;
        void ConvertRGB16_565ToARGB32(std::vector<uint32_t> &output) const;
        void ConvertARGB16_4444ToARGB32(std::vector<uint32_t> &output) const;
        void DecompressDXT1ToARGB32(std::vector<uint32_t> &output) const;
        void DecompressDXT3ToARGB32(std::vector<uint32_t> &output) const;
        void DecompressDXTBlock(std::vector<uint32_t> &output, bool hasDXT3Alpha) const;
    };

} // namespace LibOpenNFS::Shared