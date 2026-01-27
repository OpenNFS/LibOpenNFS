#include "FshTexture.h"

namespace LibOpenNFS::Shared {
    std::vector<uint32_t> FshTexture::ToARGB32() const {
        std::vector<uint32_t> pixels(static_cast<size_t>(m_width) * m_height);

        switch (m_format) {
        case PixelFormat::Indexed4Bit:
            ConvertIndexed4ToARGB32(pixels);
            break;
        case PixelFormat::Indexed8Bit:
        case PixelFormat::Indexed8BitPSH:
            ConvertIndexed8ToARGB32(pixels);
            break;
        case PixelFormat::ARGB32:
            ConvertARGB32(pixels);
            break;
        case PixelFormat::RGB24:
            ConvertRGB24ToARGB32(pixels);
            break;
        case PixelFormat::ARGB16_1555:
            ConvertARGB16_1555ToARGB32(pixels);
            break;
        case PixelFormat::ABGR16_1555:
            ConvertABGR16_1555ToARGB32(pixels);
            break;
        case PixelFormat::RGB16_565:
            ConvertRGB16_565ToARGB32(pixels);
            break;
        case PixelFormat::ARGB16_4444:
            ConvertARGB16_4444ToARGB32(pixels);
            break;
        case PixelFormat::DXT1:
            DecompressDXT1ToARGB32(pixels);
            break;
        case PixelFormat::DXT3:
            DecompressDXT3ToARGB32(pixels);
            break;
        default:
            throw std::runtime_error("Unsupported pixel format for conversion");
        }

        return pixels;
    }

    std::vector<uint8_t> FshTexture::ToRGBA() const {
        auto const argb = ToARGB32();
        std::vector<uint8_t> rgba(argb.size() * 4);

        for (size_t i = 0; i < argb.size(); ++i) {
            uint32_t const pixel = argb[i];
            rgba[i * 4 + 0] = static_cast<uint8_t>((pixel >> 16) & 0xFF); // R
            rgba[i * 4 + 1] = static_cast<uint8_t>((pixel >> 8) & 0xFF);  // G
            rgba[i * 4 + 2] = static_cast<uint8_t>(pixel & 0xFF);         // B
            rgba[i * 4 + 3] = static_cast<uint8_t>((pixel >> 24) & 0xFF); // A
        }

        return rgba;
    }

    bool FshTexture::ExportToBmp(std::string const &filepath, bool includeAlpha) const {
        std::ofstream file(filepath, std::ios::binary);
        if (!file)
            return false;

        // Convert to ARGB32
        std::vector<uint32_t> pixels = ToARGB32();

        if (includeAlpha && HasAlpha()) {
            // Export as 32-bit BGRA BMP
            size_t const rowStride = static_cast<size_t>(m_width) * 4;

            // BMP file header
            file.put('B');
            file.put('M');

            BmpFileHeader header{};
            header.headerSize = 40;
            header.size = static_cast<int32_t>(54 + rowStride * m_height);
            header.dataOffset = 54;
            header.width = m_width;
            header.height = m_height;
            header.planes = 1;
            header.bitsPerPixel = 32;
            header.imageSize = static_cast<int32_t>(rowStride * m_height);

            file.write(reinterpret_cast<char const *>(&header), sizeof(header));

            // Write pixel data (bottom-to-top) as BGRA
            std::vector<uint8_t> row(rowStride);
            for (int y = m_height - 1; y >= 0; --y) {
                for (int x = 0; x < m_width; ++x) {
                    uint32_t const pixel = pixels[y * m_width + x];
                    row[x * 4 + 0] = static_cast<uint8_t>(pixel & 0xFF);         // B
                    row[x * 4 + 1] = static_cast<uint8_t>((pixel >> 8) & 0xFF);  // G
                    row[x * 4 + 2] = static_cast<uint8_t>((pixel >> 16) & 0xFF); // R
                    row[x * 4 + 3] = static_cast<uint8_t>((pixel >> 24) & 0xFF); // A
                }
                file.write(reinterpret_cast<char const *>(row.data()), static_cast<std::streamsize>(rowStride));
            }
        } else {
            // Export as 24-bit RGB BMP (no alpha)
            size_t const rowStride = ((m_width * 3 + 3) / 4) * 4;

            // BMP file header
            file.put('B');
            file.put('M');

            BmpFileHeader header{};
            header.headerSize = 40;
            header.size = static_cast<int32_t>(54 + rowStride * m_height);
            header.dataOffset = 54;
            header.width = m_width;
            header.height = m_height;
            header.planes = 1;
            header.bitsPerPixel = 24;
            header.imageSize = static_cast<int32_t>(rowStride * m_height);

            file.write(reinterpret_cast<char const *>(&header), sizeof(header));

            // Write pixel data (bottom-to-top)
            std::vector<uint8_t> row(rowStride, 0);
            for (int y = m_height - 1; y >= 0; --y) {
                for (int x = 0; x < m_width; ++x) {
                    uint32_t const pixel = pixels[y * m_width + x];
                    row[x * 3 + 0] = static_cast<uint8_t>(pixel & 0xFF);         // B
                    row[x * 3 + 1] = static_cast<uint8_t>((pixel >> 8) & 0xFF);  // G
                    row[x * 3 + 2] = static_cast<uint8_t>((pixel >> 16) & 0xFF); // R
                }
                file.write(reinterpret_cast<char const *>(row.data()), static_cast<std::streamsize>(rowStride));
            }
        }

        return file.good();
    }

    bool FshTexture::ExportAlphaToBmp(std::string const &filepath) const {
        if (!HasAlpha())
            return false;

        std::ofstream file(filepath, std::ios::binary);
        if (!file)
            return false;

        // Get alpha channel from ARGB data
        auto const pixels = ToARGB32();

        // Calculate row stride (BMP rows must be 4-byte aligned)
        size_t const rowStride = ((m_width + 3) / 4) * 4;

        // BMP file header
        file.put('B');
        file.put('M');

        BmpFileHeader header{};
        header.headerSize = 40;
        header.size = static_cast<int32_t>(54 + 1024 + rowStride * m_height);
        header.dataOffset = 54 + 1024;
        header.width = m_width;
        header.height = m_height;
        header.planes = 1;
        header.bitsPerPixel = 8;
        header.colorsUsed = 256;
        header.imageSize = static_cast<int32_t>(rowStride * m_height);

        file.write(reinterpret_cast<char const *>(&header), sizeof(header));

        // Write grayscale palette
        for (int i = 0; i < 256; ++i) {
            uint8_t const gray = static_cast<uint8_t>(i);
            file.put(static_cast<char>(gray)); // B
            file.put(static_cast<char>(gray)); // G
            file.put(static_cast<char>(gray)); // R
            file.put(0);                       // Reserved
        }

        // Write alpha data (bottom-to-top)
        std::vector<uint8_t> row(rowStride, 0);
        for (int y = m_height - 1; y >= 0; --y) {
            for (int x = 0; x < m_width; ++x) {
                row[x] = static_cast<uint8_t>((pixels[y * m_width + x] >> 24) & 0xFF);
            }
            file.write(reinterpret_cast<char const *>(row.data()), static_cast<std::streamsize>(rowStride));
        }

        return file.good();
    }

    void FshTexture::ConvertIndexed4ToARGB32(std::vector<uint32_t> &output) const {
        // For PSH 4-bit indexed, raw data is already unpacked to 1 byte per pixel
        for (size_t i = 0; i < m_rawData.size() && i < output.size(); ++i) {
            uint8_t const index = m_rawData[i] & 0x0F; // Ensure index is in valid range
            output[i] = m_palette[index].ToARGB32();
        }
    }

    void FshTexture::ConvertIndexed8ToARGB32(std::vector<uint32_t> &output) const {
        for (size_t i = 0; i < m_rawData.size(); ++i) {
            output[i] = m_palette[m_rawData[i]].ToARGB32();
        }
    }

    void FshTexture::ConvertARGB32(std::vector<uint32_t> &output) const {
        for (size_t y = 0; y < m_height; ++y) {
            for (size_t x = 0; x < m_width; ++x) {
                size_t const srcIdx = (y * m_width + x) * 4;
                size_t const dstIdx = y * m_width + x;
                uint8_t const b = m_rawData[srcIdx];
                uint8_t const g = m_rawData[srcIdx+1];
                uint8_t const r = m_rawData[srcIdx+2];
                uint8_t const a = m_rawData[srcIdx+3];
                output[dstIdx] = (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) |
                              static_cast<uint32_t>(b);
            }
        }
    }

    void FshTexture::ConvertRGB24ToARGB32(std::vector<uint32_t> &output) const {
        for (size_t y = 0; y < m_height; ++y) {
            for (size_t x = 0; x < m_width; ++x) {
                size_t const srcIdx = (y * m_width + x) * 3;
                size_t const dstIdx = y * m_width + x;
                uint8_t const b = m_rawData[srcIdx+0];
                uint8_t const g = m_rawData[srcIdx+1];
                uint8_t const r = m_rawData[srcIdx+2];
                output[dstIdx] = 0xFF000000 | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) |
                              static_cast<uint32_t>(b);
            }
        }
    }

    void FshTexture::ConvertARGB16_1555ToARGB32(std::vector<uint32_t> &output) const {
        auto const *src = reinterpret_cast<uint16_t const *>(m_rawData.data());
        for (size_t i = 0; i < static_cast<size_t>(m_width) * m_height; ++i) {
            output[i] = Colour::FromARGB16_1555(src[i]).ToARGB32();
        }
    }

    void FshTexture::ConvertABGR16_1555ToARGB32(std::vector<uint32_t> &output) const {
        auto const *src = reinterpret_cast<uint16_t const *>(m_rawData.data());
        for (size_t i = 0; i < static_cast<size_t>(m_width) * m_height; ++i) {
            output[i] = Colour::FromABGR16_1555(src[i]).ToARGB32();
        }
    }

    void FshTexture::ConvertRGB16_565ToARGB32(std::vector<uint32_t> &output) const {
        auto const *src = reinterpret_cast<uint16_t const *>(m_rawData.data());
        for (size_t i = 0; i < static_cast<size_t>(m_width) * m_height; ++i) {
            output[i] = Colour::FromRGB16_565(src[i]).ToARGB32();
        }
    }

    void FshTexture::ConvertARGB16_4444ToARGB32(std::vector<uint32_t> &output) const {
        auto const *src = reinterpret_cast<uint16_t const *>(m_rawData.data());
        for (size_t i = 0; i < static_cast<size_t>(m_width) * m_height; ++i) {
            output[i] = Colour::FromARGB16_4444(src[i]).ToARGB32();
        }
    }

    void FshTexture::DecompressDXT1ToARGB32(std::vector<uint32_t> &output) const {
        DecompressDXTBlock(output, false);
    }

    void FshTexture::DecompressDXT3ToARGB32(std::vector<uint32_t> &output) const {
        DecompressDXTBlock(output, true);
    }

    void FshTexture::DecompressDXTBlock(std::vector<uint32_t> &output, bool const hasDXT3Alpha) const {
        size_t const blockWidth = (m_width + 3) / 4;
        size_t const blockHeight = (m_height + 3) / 4;
        size_t const blockSize = hasDXT3Alpha ? 16 : 8;

        for (size_t by = 0; by < blockHeight; ++by) {
            for (size_t bx = 0; bx < blockWidth; ++bx) {
                uint8_t const *blockData = m_rawData.data() + (by * blockWidth + bx) * blockSize;

                // DXT3: First 8 bytes are alpha, then colour
                uint8_t const *alphaData = hasDXT3Alpha ? blockData : nullptr;
                uint8_t const *colorData = hasDXT3Alpha ? blockData + 8 : blockData;

                // Decode 2 16-bit colors
                uint16_t const c0 = static_cast<uint16_t>(colorData[0]) | (static_cast<uint16_t>(colorData[1]) << 8);
                uint16_t const c1 = static_cast<uint16_t>(colorData[2]) | (static_cast<uint16_t>(colorData[3]) << 8);

                Colour colors[4];
                colors[0] = Colour::FromRGB16_565(c0);
                colors[1] = Colour::FromRGB16_565(c1);

                if (c0 > c1) {
                    // 4-color mode
                    colors[2].r = static_cast<uint8_t>((2 * colors[0].r + colors[1].r) / 3);
                    colors[2].g = static_cast<uint8_t>((2 * colors[0].g + colors[1].g) / 3);
                    colors[2].b = static_cast<uint8_t>((2 * colors[0].b + colors[1].b) / 3);
                    colors[2].a = 255;
                    colors[3].r = static_cast<uint8_t>((colors[0].r + 2 * colors[1].r) / 3);
                    colors[3].g = static_cast<uint8_t>((colors[0].g + 2 * colors[1].g) / 3);
                    colors[3].b = static_cast<uint8_t>((colors[0].b + 2 * colors[1].b) / 3);
                    colors[3].a = 255;
                } else {
                    // 3-color + transparent mode
                    colors[2].r = static_cast<uint8_t>((colors[0].r + colors[1].r) / 2);
                    colors[2].g = static_cast<uint8_t>((colors[0].g + colors[1].g) / 2);
                    colors[2].b = static_cast<uint8_t>((colors[0].b + colors[1].b) / 2);
                    colors[2].a = 255;
                    colors[3] = Colour(0, 0, 0, 0); // Transparent
                }

                // Decode 4x4 pixel block
                uint32_t const colorIndices = static_cast<uint32_t>(colorData[4]) | (static_cast<uint32_t>(colorData[5]) << 8) |
                                              (static_cast<uint32_t>(colorData[6]) << 16) | (static_cast<uint32_t>(colorData[7]) << 24);

                for (size_t py = 0; py < 4; ++py) {
                    for (size_t px = 0; px < 4; ++px) {
                        size_t const x = bx * 4 + px;
                        size_t const y = by * 4 + py;

                        if (x < m_width && y < m_height) {
                            size_t const bitOffset = (py * 4 + px) * 2;
                            uint8_t const colorIndex = (colorIndices >> bitOffset) & 0x03;

                            Colour pixel = colors[colorIndex];

                            // Apply DXT3 alpha if present
                            if (hasDXT3Alpha) {
                                size_t const alphaByteIdx = py * 2 + (px / 2);
                                uint8_t const alphaNibble = (px & 1) ? (alphaData[alphaByteIdx] >> 4) : (alphaData[alphaByteIdx] & 0x0F);
                                pixel.a = static_cast<uint8_t>(alphaNibble * 17);
                            }

                            output[y * m_width + x] = pixel.ToARGB32();
                        }
                    }
                }
            }
        }
    }

} // namespace LibOpenNFS::Shared