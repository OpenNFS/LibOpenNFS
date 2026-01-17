#include "FshArchive.h"

namespace LibOpenNFS::Shared {
    bool FshArchive::Load(std::string const &filepath) {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file) {
            m_lastError = "Failed to open file: " + filepath;
            return false;
        }

        auto const fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        m_rawData.resize(static_cast<size_t>(fileSize));
        if (!file.read(reinterpret_cast<char *>(m_rawData.data()), fileSize)) {
            m_lastError = "Failed to read file: " + filepath;
            return false;
        }

        m_sourcePath = filepath;
        return ParseData();
    }

    bool FshArchive::Load(std::vector<uint8_t> const &data) {
        m_rawData = data;
        m_sourcePath.clear();
        return ParseData();
    }

    bool FshArchive::Load(std::vector<uint8_t> &&data) {
        m_rawData = std::move(data);
        m_sourcePath.clear();
        return ParseData();
    }

    FshTexture const *FshArchive::GetTexture(std::string const &name) const {
        auto const it = m_textureMap.find(name);
        return it != m_textureMap.end() ? &m_textures[it->second] : nullptr;
    }

    FshTexture const &FshArchive::GetTexture(size_t const index) const {
        return m_textures.at(index);
    }

    bool FshArchive::ExtractAll(std::string const &outputDir, bool const preserveNames, bool const combineAlpha) const {
        if (!std::filesystem::exists(outputDir)) {
            std::filesystem::create_directories(outputDir);
        }

        for (size_t i = 0; i < m_textures.size(); ++i) {
            auto const &tex = m_textures[i];

            std::string filename;
            if (preserveNames && IsValidFilename(tex.Name())) {
                filename = tex.Name() + ".BMP";
            } else {
                char buf[16];
                std::snprintf(buf, sizeof(buf), "%04zu.BMP", i);
                filename = buf;
            }

            std::string const filepath = outputDir + "/" + filename;
            if (!tex.ExportToBmp(filepath, combineAlpha)) {
                m_lastError = "Failed to export texture: " + filepath;
                return false;
            }

            // Export separate alpha channel if requested and texture has alpha
            if (!combineAlpha && tex.HasAlpha()) {
                std::string alphaFilename;
                if (preserveNames && IsValidFilename(tex.Name())) {
                    alphaFilename = tex.Name() + "-a.BMP";
                } else {
                    char buf[16];
                    std::snprintf(buf, sizeof(buf), "%04zu-a.BMP", i);
                    alphaFilename = buf;
                }
                tex.ExportAlphaToBmp(outputDir + "/" + alphaFilename);
            }
        }

        return true;
    }

    bool FshArchive::ParseData() {
        if (m_rawData.size() < 16) {
            m_lastError = "File too small to be a valid FSH/QFS archive";
            return false;
        }

        // Check for QFS compression and decompress if needed
        if (QfsCompression::IsCompressed(m_rawData)) {
            m_wasCompressed = true;
            try {
                m_fshData = QfsCompression::Decompress(m_rawData);
            } catch (std::exception const &e) {
                m_lastError = std::string("QFS decompression failed: ") + e.what();
                return false;
            }
        } else {
            m_wasCompressed = false;
            m_fshData = m_rawData;
        }

        // Validate FSH header
        auto const *header = reinterpret_cast<FshHeader const *>(m_fshData.data());
        if (std::strncmp(header->magic, FSH_MAGIC, 4) != 0 && std::strncmp(header->magic, FSH_MAGIC_ALT, 4) != 0) {
            m_lastError = "Invalid FSH magic bytes";
            return false;
        }

        m_directoryId = std::string(header->directoryId, 4);

        // Parse directory entries
        size_t const numEntries = static_cast<size_t>(header->numEntries);
        if (m_fshData.size() < 16 + numEntries * 8) {
            m_lastError = "File too small for directory entries";
            return false;
        }

        auto const *directory = reinterpret_cast<FshDirectoryEntry const *>(m_fshData.data() + 16);

        // First pass: Find global palette
        for (size_t i = 0; i < numEntries; ++i) {
            std::string const name(directory[i].name, 4);
            if (name == "!pal") {
                ParsePalette(directory[i].offset, m_globalPalette);
                m_hasGlobalPalette = true;
                break;
            }
        }

        // If no !pal entry, look for any palette entry
        if (!m_hasGlobalPalette) {
            for (size_t i = 0; i < numEntries; ++i) {
                size_t const offset = static_cast<size_t>(directory[i].offset);
                if (offset + 16 > m_fshData.size())
                    continue;

                auto const *entryHeader = reinterpret_cast<FshEntryHeader const *>(m_fshData.data() + offset);
                uint8_t const code = entryHeader->GetFormatCode();
                if (IsPaletteCode(code)) {
                    ParsePalette(offset, m_globalPalette);
                    m_hasGlobalPalette = true;
                    break;
                }
            }
        }

        // Second pass: Parse all texture entries
        m_textures.clear();
        m_textureMap.clear();

        for (size_t i = 0; i < numEntries; ++i) {
            std::string const name(directory[i].name, 4);
            size_t const offset = static_cast<size_t>(directory[i].offset);

            // Find next entry offset for bounds checking
            size_t nextOffset = static_cast<size_t>(header->fileSize);
            for (size_t j = 0; j < numEntries; ++j) {
                if (static_cast<size_t>(directory[j].offset) > offset && static_cast<size_t>(directory[j].offset) < nextOffset) {
                    nextOffset = static_cast<size_t>(directory[j].offset);
                }
            }

            if (offset + 16 > m_fshData.size())
                continue;

            auto const *entryHeader = reinterpret_cast<FshEntryHeader const *>(m_fshData.data() + offset);
            uint8_t const code = entryHeader->GetFormatCode() & 0x7F;

            // Check if this is a bitmap entry
            if (IsBitmapCode(code)) {
                FshTexture texture = ParseTexture(name, offset, nextOffset);
                m_textureMap[texture.Name()] = m_textures.size();
                m_textures.push_back(std::move(texture));
            }
        }

        return true;
    }

    bool FshArchive::IsBitmapCode(uint8_t const code) {
        return code == 0x78 || code == 0x7B || code == 0x7D || code == 0x7E || code == 0x7F || code == 0x6D || code == 0x60 || code == 0x61;
    }

    bool FshArchive::IsPaletteCode(uint8_t const code) {
        return code == 0x22 || code == 0x24 || code == 0x29 || code == 0x2A || code == 0x2D;
    }

    void FshArchive::ParsePalette(size_t const offset, Palette &palette) const {
        auto const *header = reinterpret_cast<FshEntryHeader const *>(m_fshData.data() + offset);
        uint8_t const code = header->GetFormatCode();
        size_t const numColors = static_cast<size_t>(header->width);
        uint8_t const *data = m_fshData.data() + offset + 16;

        palette.Resize(numColors);

        switch (code) {
        case 0x24: // RGB24
            for (size_t i = 0; i < numColors; ++i) {
                palette[i] = Colour(data[i * 3], data[i * 3 + 1], data[i * 3 + 2], 255);
            }
            break;
        case 0x22: // DOS RGB24 (6-bit per channel)
            for (size_t i = 0; i < numColors; ++i) {
                palette[i] = Colour(static_cast<uint8_t>(data[i * 3] << 2), static_cast<uint8_t>(data[i * 3 + 1] << 2),
                                    static_cast<uint8_t>(data[i * 3 + 2] << 2), 255);
            }
            break;
        case 0x2D: { // ARGB16 1555
            auto const *data16 = reinterpret_cast<uint16_t const *>(data);
            for (size_t i = 0; i < numColors; ++i) {
                uint16_t const val = data16[i];
                palette[i] = Colour(static_cast<uint8_t>((val & 0x1F) << 3), static_cast<uint8_t>(((val >> 5) & 0x1F) << 3),
                                    static_cast<uint8_t>(((val >> 10) & 0x1F) << 3), static_cast<uint8_t>((val & 0x8000) ? 255 : 0));
            }
            break;
        }
        case 0x29: { // RGB16 565
            auto const *data16 = reinterpret_cast<uint16_t const *>(data);
            for (size_t i = 0; i < numColors; ++i) {
                uint16_t const val = data16[i];
                palette[i] = Colour(static_cast<uint8_t>((val & 0x1F) << 3), static_cast<uint8_t>(((val >> 5) & 0x3F) << 2),
                                    static_cast<uint8_t>(((val >> 11) & 0x1F) << 3), 255);
            }
            break;
        }
        case 0x2A: { // ARGB32
            auto const *data32 = reinterpret_cast<uint32_t const *>(data);
            for (size_t i = 0; i < numColors; ++i) {
                palette[i] = Colour::FromARGB32(data32[i]);
            }
            break;
        }
        default:
            throw std::runtime_error("Unsupported pixel format for palette");
        }
    }

    FshTexture FshArchive::ParseTexture(std::string const &name, size_t const offset, size_t const nextOffset) const {
        auto const *header = reinterpret_cast<FshEntryHeader const *>(m_fshData.data() + offset);
        uint8_t const rawCode = header->GetFormatCode();
        uint8_t const code = rawCode & 0x7F;
        bool const isCompressed = (rawCode & 0x80) != 0;

        auto const format = static_cast<PixelFormat>(code);
        FshTexture texture(name, header->width, header->height, format);

        // Calculate pixel data size
        size_t dataSize = 0;
        switch (format) {
        case PixelFormat::Indexed8Bit:
            dataSize = static_cast<size_t>(header->width) * header->height;
            break;
        case PixelFormat::ARGB32:
            dataSize = static_cast<size_t>(header->width) * header->height * 4;
            break;
        case PixelFormat::RGB24:
            dataSize = static_cast<size_t>(header->width) * header->height * 3;
            break;
        case PixelFormat::ARGB16_1555:
        case PixelFormat::RGB16_565:
        case PixelFormat::ARGB16_4444:
            dataSize = static_cast<size_t>(header->width) * header->height * 2;
            break;
        case PixelFormat::DXT1:
            dataSize = static_cast<size_t>((header->width + 3) / 4) * ((header->height + 3) / 4) * 8;
            break;
        case PixelFormat::DXT3:
            dataSize = static_cast<size_t>((header->width + 3) / 4) * ((header->height + 3) / 4) * 16;
            break;
        default:
            break;
        }

        // Extract pixel data
        size_t const pixelDataOffset = offset + 16;

        if (isCompressed) {
            // Entry-level compression (rare)
            size_t const compressedSize =
                (header->GetNextOffset() > 0) ? static_cast<size_t>(header->GetNextOffset()) - 16 : nextOffset - pixelDataOffset;

            try {
                auto decompressed = QfsCompression::Decompress(m_fshData.data() + pixelDataOffset, compressedSize);
                texture.RawData() = std::move(decompressed);
            } catch (...) {
                // Fall back to raw data
                texture.RawData().resize(dataSize);
                std::memcpy(texture.RawData().data(), m_fshData.data() + pixelDataOffset, std::min(dataSize, nextOffset - pixelDataOffset));
            }
        } else {
            texture.RawData().resize(dataSize);
            if (pixelDataOffset + dataSize <= m_fshData.size()) {
                std::memcpy(texture.RawData().data(), m_fshData.data() + pixelDataOffset, dataSize);
            }
        }

        // Parse attachments (local palette, alpha, etc.)
        size_t attachOffset = offset;
        auto const *attachHeader = header;

        while (attachHeader->GetNextOffset() > 0) {
            attachOffset += static_cast<size_t>(attachHeader->GetNextOffset());
            if (attachOffset + 16 > m_fshData.size())
                break;

            attachHeader = reinterpret_cast<FshEntryHeader const *>(m_fshData.data() + attachOffset);
            uint8_t const attachCode = attachHeader->GetFormatCode();

            // Check for local palette
            if (IsPaletteCode(attachCode) && format == PixelFormat::Indexed8Bit) {
                ParsePalette(attachOffset, texture.GetPalette());
            }
        }

        // Apply global palette if no local palette and format requires it
        if (format == PixelFormat::Indexed8Bit && texture.GetPalette().Colors()[0].a == 0 && m_hasGlobalPalette) {
            texture.GetPalette() = m_globalPalette;
        }

        // Set alpha flag based on format
        texture.SetHasAlphaAttachment(HasAlphaChannel(format));

        return texture;
    }

    bool FshArchive::IsValidFilename(std::string const &name) {
        if (name.empty() || name.length() > 4)
            return false;
        for (char const c : name) {
            if (!std::isalnum(static_cast<unsigned char>(c)))
                return false;
        }
        return true;
    }

} // namespace LibOpenNFS::Shared