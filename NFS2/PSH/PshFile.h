#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS::NFS2 {
    class PshFile : IRawData {
        struct HEADER {
            char header[4];        //  "SHPP"
            uint32_t length;       // Inclusive Length of the PSH file
            uint32_t nDirectories; // Number of directory entries
            char chk[4];           // "GIMX"
        };

        struct DIR_ENTRY {
            char imageName[4];
            uint32_t imageOffset; // Offset to start of the image, len implied by difference between offsets to next
        };

        struct IMAGE_HEADER {
            uint8_t imageType; // Image type: Observed values are 0x40, 0x42, 0x43, and 0xC0 The bottom 2 bits of the image type byte specify
            // the bit depth of the image: 0 - 4-bit indexed colour 2 - 16-bit direct colour 3 - 24-bit direct colour
            uint8_t unknown[3];
            uint16_t width;
            uint16_t height;
            uint16_t unknown2[4];
        };

        struct PALETTE_HEADER {
            uint32_t unknown;
            uint16_t paletteWidth;    // Always 16
            uint16_t paletteHeight;   // Always 1
            uint16_t nPaletteEntries; // Always 16
            uint16_t unknown2[3];     // [0] always 0 [1] always 0 [2] often 240, sometimes 0
        };

      public:
        PshFile() = default;

        static bool Load(std::string const &pshPath, PshFile &pshFile);
        static void Save(std::string const &pshPath, PshFile &pshFile);
        static bool Extract(std::string const &outputPath, PshFile &pshFile);
        // TODO: Remove these, pure dirt to restore PS1 tex load temporarily
        // Define Windows Bitmap structs and macros with CP (CrossPlatform prefix) to avoid redef when conditionally including Windows.h for logging
        // colour handles
#define CP_BI_RGB 0x0000

#pragma pack(push, 2)
        typedef struct tagCP_BITMAPFILEHEADER {
            uint16_t bfType;
            uint32_t bfSize;
            uint16_t bfReserved1;
            uint16_t bfReserved2;
            uint32_t bfOffBits;
        } CP_BITMAPFILEHEADER;
#pragma pack(pop)

        typedef struct tagCP_BITMAPINFOHEADER {
            uint32_t biSize;
            int32_t biWidth;
            int32_t biHeight;
            uint16_t biPlanes;
            uint16_t biBitCount;
            uint32_t biCompression;
            uint32_t biSizeImage;
            int32_t biXPelsPerMeter;
            int32_t biYPelsPerMeter;
            uint32_t biClrUsed;
            uint32_t biClrImportant;
        } CP_BITMAPINFOHEADER, *PCP_BITMAPINFOHEADER;

        typedef struct tagCP_RGBQUAD {
            uint8_t rgbBlue;
            uint8_t rgbGreen;
            uint8_t rgbRed;
            uint8_t rgbReserved;
        } CP_RGBQUAD;

        typedef struct tagCP_BITMAPINFO {
            CP_BITMAPINFOHEADER bmiHeader;
            CP_RGBQUAD bmiColors[1];
        } CP_BITMAPINFO;

        static bool SaveImage(char const *szPathName, void const *lpBits, uint16_t const w, uint16_t const h);
        static bool Extract(std::string const &inputPath, std::string const &outputPath);

        HEADER header;
        std::vector<DIR_ENTRY> directoryEntries;

      private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS::NFS2
