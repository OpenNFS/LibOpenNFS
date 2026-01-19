#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace LibOpenNFS::Shared {

    // FSH/QFS Magic identifiers
    constexpr char FSH_MAGIC[] = "SHPI";
    constexpr char FSH_MAGIC_ALT[] = "SHPP";
    constexpr uint8_t QFS_MAGIC_BYTE0 = 0x10;
    constexpr uint8_t QFS_MAGIC_BYTE0_ALT = 0x11;
    constexpr uint8_t QFS_MAGIC_BYTE1 = 0xFB;

    // Pixel format codes used in FSH entries
    enum class PixelFormat : uint8_t {
        Indexed4Bit = 0x40,    // 4-bit indexed with palette (PSH)
        Indexed8BitPSH = 0x41, // 8-bit indexed with palette (PSH)
        Indexed8Bit = 0x7B,    // 8-bit indexed with palette
        ARGB32 = 0x7D,         // 32-bit ARGB (8:8:8:8)
        RGB24 = 0x7F,          // 24-bit RGB (0:8:8:8)
        ARGB16_1555 = 0x7E,    // 16-bit ARGB (1:5:5:5)
        ABGR16_1555 = 0x79,    // 16-bit ABGR (1:5:5:5) - PlayStation format
        RGB16_565 = 0x78,      // 16-bit RGB (0:5:6:5)
        ARGB16_4444 = 0x6D,    // 16-bit ARGB (4:4:4:4)
        DXT1 = 0x60,           // DXT1 compressed (NFS6+)
        DXT3 = 0x61,           // DXT3 compressed (NFS6+)
        Unknown = 0xFF
    };

    // Palette format codes
    enum class PaletteFormat : uint8_t {
        RGB24 = 0x24,       // 24-bit RGB palette
        RGB24_DOS = 0x22,   // 24-bit DOS palette (6-bit per channel)
        ARGB16_1555 = 0x2D, // 16-bit ARGB (1:5:5:5) palette
        RGB16_565 = 0x29,   // 16-bit RGB (5:6:5) palette (NFS5+)
        ARGB32 = 0x2A,      // 32-bit ARGB palette
        Unknown = 0xFF
    };

    // Attachment types
    enum class AttachmentType : uint8_t {
        Text = 0x6F,           // Text attachment
        ExtendedText = 0x69,   // Extended text (NFS6+)
        ExtendedText70 = 0x70, // Another extended text format
        Binary = 0x00          // Generic binary data
    };

    // Bits per pixel for each format
    inline uint8_t GetBitsPerPixel(PixelFormat const format) {
        switch (format) {
        case PixelFormat::Indexed4Bit:
            return 4;
        case PixelFormat::Indexed8Bit:
        case PixelFormat::Indexed8BitPSH:
            return 8;
        case PixelFormat::ARGB32:
            return 32;
        case PixelFormat::RGB24:
            return 24;
        case PixelFormat::ARGB16_1555:
        case PixelFormat::ABGR16_1555:
        case PixelFormat::RGB16_565:
        case PixelFormat::ARGB16_4444:
            return 16;
        case PixelFormat::DXT1:
            return 4; // Compressed
        case PixelFormat::DXT3:
            return 8; // Compressed
        default:
            return 0;
        }
    }

    inline bool HasAlphaChannel(PixelFormat const format) {
        switch (format) {
        case PixelFormat::ARGB32:
        case PixelFormat::ARGB16_1555:
        case PixelFormat::ABGR16_1555:
        case PixelFormat::ARGB16_4444:
        case PixelFormat::DXT3:
            return true;
        default:
            return false;
        }
    }

    inline bool IsCompressedFormat(PixelFormat const format) {
        return format == PixelFormat::DXT1 || format == PixelFormat::DXT3;
    }

    inline std::string PixelFormatToString(PixelFormat const format) {
        switch (format) {
        case PixelFormat::Indexed4Bit:
            return "4-bit Indexed";
        case PixelFormat::Indexed8Bit:
        case PixelFormat::Indexed8BitPSH:
            return "8-bit Indexed";
        case PixelFormat::ARGB32:
            return "32-bit ARGB";
        case PixelFormat::RGB24:
            return "24-bit RGB";
        case PixelFormat::ARGB16_1555:
            return "16-bit ARGB (1:5:5:5)";
        case PixelFormat::ABGR16_1555:
            return "16-bit ABGR (1:5:5:5)";
        case PixelFormat::RGB16_565:
            return "16-bit RGB (5:6:5)";
        case PixelFormat::ARGB16_4444:
            return "16-bit ARGB (4:4:4:4)";
        case PixelFormat::DXT1:
            return "DXT1";
        case PixelFormat::DXT3:
            return "DXT3";
        default:
            return "Unknown";
        }
    }

// Raw FSH file header (16 bytes)
#pragma pack(push, 1)
    struct FshHeader {
        char magic[4];       // "SHPI" or "SHPP"
        int32_t fileSize;    // Total file size
        int32_t numEntries;  // Number of directory entries
        char directoryId[4]; // Directory identifier
    };

    // FSH directory entry (8 bytes)
    struct FshDirectoryEntry {
        char name[4];   // Entry name (4 characters)
        int32_t offset; // Offset to entry data
    };

    // FSH entry header (16 bytes)
    struct FshEntryHeader {
        int32_t code; // Format code (low byte) and next attachment offset (high 3 bytes)
        int16_t width;
        int16_t height;
        int16_t misc[4]; // Format-specific data

        uint8_t GetFormatCode() const {
            return static_cast<uint8_t>(code & 0xFF);
        }
        bool IsCompressed() const {
            return (code & 0x80) != 0;
        }
        int32_t GetNextOffset() const {
            return code >> 8;
        }
    };

    // BMP file header for export
    struct BmpFileHeader {
        int32_t size;
        int32_t reserved;
        int32_t dataOffset;
        int32_t headerSize;
        int32_t width;
        int32_t height;
        int16_t planes;
        int16_t bitsPerPixel;
        int32_t compression;
        int32_t imageSize;
        int32_t xPixelsPerMeter;
        int32_t yPixelsPerMeter;
        int32_t colorsUsed;
        int32_t colorsImportant;
    };
#pragma pack(pop)

    // Represents a colour in ARGB format
    struct Colour {
        uint8_t r, g, b, a;

        Colour() : r(0), g(0), b(0), a(255) {
        }
        Colour(uint8_t const r, uint8_t const g, uint8_t const b, uint8_t const a = 255) : r(r), g(g), b(b), a(a) {
        }

        static Colour FromARGB32(uint32_t const argb) {
            return Colour(static_cast<uint8_t>((argb >> 16) & 0xFF), static_cast<uint8_t>((argb >> 8) & 0xFF),
                          static_cast<uint8_t>(argb & 0xFF), static_cast<uint8_t>((argb >> 24) & 0xFF));
        }

        static Colour FromRGB16_565(uint16_t const rgb) {
            // RGB565: bits 11-15 = R, bits 5-10 = G, bits 0-4 = B
            return Colour(static_cast<uint8_t>(((rgb >> 11) & 0x1F) << 3), static_cast<uint8_t>(((rgb >> 5) & 0x3F) << 2),
                          static_cast<uint8_t>((rgb & 0x1F) << 3), 255);
        }

        static Colour FromARGB16_1555(uint16_t const argb) {
            // ARGB1555: bit 15 = A, bits 10-14 = R, bits 5-9 = G, bits 0-4 = B
            return Colour(static_cast<uint8_t>(((argb >> 10) & 0x1F) << 3), static_cast<uint8_t>(((argb >> 5) & 0x1F) << 3),
                          static_cast<uint8_t>((argb & 0x1F) << 3), static_cast<uint8_t>((argb & 0x8000) ? 255 : 0));
        }

        static Colour FromABGR16_1555(uint16_t const abgr) {
            // ABGR1555 (PlayStation): bit 15 = A, bits 10-14 = B, bits 5-9 = G, bits 0-4 = R
            auto const r = static_cast<uint8_t>(((abgr & 0x1F) * 255 + 15) / 31);
            auto const g = static_cast<uint8_t>((((abgr >> 5) & 0x1F) * 255 + 15) / 31);
            auto const b = static_cast<uint8_t>((((abgr >> 10) & 0x1F) * 255 + 15) / 31);
            // Alpha: transparent if A bit is 0 AND color is black, otherwise opaque
            uint8_t a = 255;
            if (((abgr & 0x8000) == 0) && (r == 0) && (g == 0) && (b == 0)) {
                a = 0;
            }
            return Colour(r, g, b, a);
        }

        static Colour FromARGB16_4444(uint16_t const argb) {
            // ARGB4444: bits 12-15 = A, bits 8-11 = R, bits 4-7 = G, bits 0-3 = B
            return Colour(static_cast<uint8_t>(((argb >> 8) & 0x0F) * 17), static_cast<uint8_t>(((argb >> 4) & 0x0F) * 17),
                          static_cast<uint8_t>((argb & 0x0F) * 17), static_cast<uint8_t>(((argb >> 12) & 0x0F) * 17));
        }

        uint32_t ToARGB32() const {
            return (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) |
                   static_cast<uint32_t>(b);
        }

        uint32_t ToBGRA32() const {
            return (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) |
                   static_cast<uint32_t>(b);
        }
    };

    // Palette for indexed formats
    class Palette {
      public:
        static constexpr size_t MAX_COLORS = 256;

        Palette() : m_colors(MAX_COLORS) {
        }

        Colour &operator[](size_t const index) {
            return m_colors[index];
        }
        Colour const &operator[](size_t const index) const {
            return m_colors[index];
        }

        size_t Size() const {
            return m_colors.size();
        }
        void Resize(size_t const size) {
            m_colors.resize(size);
        }

        std::vector<Colour> const &Colors() const {
            return m_colors;
        }
        std::vector<Colour> &Colors() {
            return m_colors;
        }

      private:
        std::vector<Colour> m_colors;
    };

} // namespace LibOpenNFS::Shared