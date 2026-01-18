#pragma once

#include "FshTexture.h"
#include "FshTypes.h"
#include "QfsCompression.h"

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace LibOpenNFS::Shared {

    /**
     * Represents an FSH/QFS texture archive.
     *
     * FSH (SHPI) archives are used by EA's Need for Speed games to store textures.
     * QFS files are QFS-compressed FSH files.
     *
     * Usage:
     *   FshArchive archive;
     *   if (archive.Load("textures.qfs")) {
     *       for (const auto& tex : archive.Textures()) {
     *           tex.ExportToBmp("output/" + tex.Name() + ".bmp");
     *       }
     *   }
     */
    class FshArchive {
      public:
        FshArchive() = default;

        /**
         * Load an FSH or QFS archive from file
         * @param filepath Path to the archive file
         * @param skipMirroredImages If true, skip mirrored/duplicate images (used for NFS4 track textures)
         * @return true on success
         */
        bool Load(std::string const &filepath, bool skipMirroredImages = false);

        /**
         * Load an FSH or QFS archive from memory
         * @param data Raw archive data
         * @param skipMirroredImages If true, skip mirrored/duplicate images (used for NFS4 track textures)
         * @return true on success
         */
        bool Load(std::vector<uint8_t> const &data, bool skipMirroredImages = false);
        bool Load(std::vector<uint8_t> &&data, bool skipMirroredImages = false);

        /**
         * Check if archive was compressed (QFS)
         */
        bool WasCompressed() const {
            return m_wasCompressed;
        }

        /**
         * Get the directory ID from the FSH header
         */
        std::string const &DirectoryId() const {
            return m_directoryId;
        }

        /**
         * Get all textures in the archive
         */
        std::vector<FshTexture> const &Textures() const {
            return m_textures;
        }

        /**
         * Get a texture by name
         * @param name 4-character texture name
         * @return Pointer to texture or nullptr if not found
         */
        FshTexture const *GetTexture(std::string const &name) const;

        /**
         * Get a texture by index
         */
        FshTexture const &GetTexture(size_t index) const;

        /**
         * Get the number of textures
         */
        size_t TextureCount() const {
            return m_textures.size();
        }

        /**
         * Extract all textures to a directory as BMP files
         * @param outputDir Output directory path
         * @param preserveNames If true, use original 4-char names; if false, use numbered names
         * @param combineAlpha If true, export as 32-bit BGRA with alpha embedded; if false, export separate alpha files
         * @return true on success
         */
        bool ExtractAll(std::string const &outputDir, bool preserveNames = false, bool combineAlpha = true) const;

        /**
         * Get the last error message
         */
        std::string const &LastError() const {
            return m_lastError;
        }

        /**
         * Get the global palette if one exists
         */
        Palette const *GlobalPalette() const {
            return m_hasGlobalPalette ? &m_globalPalette : nullptr;
        }

      private:
        std::vector<uint8_t> m_rawData;
        std::vector<uint8_t> m_fshData; // Decompressed FSH data
        std::string m_sourcePath;
        std::string m_directoryId;
        std::vector<FshTexture> m_textures;
        std::unordered_map<std::string, size_t> m_textureMap;
        Palette m_globalPalette;
        bool m_hasGlobalPalette = false;
        bool m_wasCompressed = false;
        bool m_skipMirroredImages = false;
        mutable std::string m_lastError;

        bool ParseData();
        static bool IsBitmapCode(uint8_t code);
        static bool IsPaletteCode(uint8_t code);
        void ParsePalette(size_t offset, Palette &palette) const;
        FshTexture ParseTexture(std::string const &name, size_t offset, size_t nextOffset) const;
        static bool IsValidFilename(std::string const &name);
    };

} // namespace LibOpenNFS::Shared