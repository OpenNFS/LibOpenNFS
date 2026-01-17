#pragma once

#include <vector>

namespace LibOpenNFS::Shared {

    /**
     * QFS Compression/Decompression
     *
     * QFS is an LZ77-style compression format used by EA games.
     * The format consists of:
     * - 2-byte header identifying compression type (0x10FB or 0x11FB)
     * - 3-byte uncompressed size (big-endian)
     * - Optional 3-byte compressed size (if header byte 0 has bit 0 set)
     * - Compressed data stream
     */
    class QfsCompression {
      public:
        // Quality factor for compression (higher = better but slower)
        static constexpr int DEFAULT_MAX_ITERATIONS = 50;

        /**
         * Check if data is QFS compressed
         * @param data Input data
         * @param size Size of data
         * @return true if data appears to be QFS compressed
         */
        static bool IsCompressed(uint8_t const *data, size_t size);
        static bool IsCompressed(std::vector<uint8_t> const &data);

        /**
         * Get the uncompressed size from QFS header
         * @param data QFS compressed data
         * @return Uncompressed size
         */
        static size_t GetUncompressedSize(uint8_t const *data);

        /**
         * Decompress QFS data
         * @param input Compressed data
         * @param inputSize Size of compressed data
         * @return Decompressed data
         * @throws std::runtime_error on decompression failure
         */
        static std::vector<uint8_t> Decompress(uint8_t const *input, size_t inputSize);
        static std::vector<uint8_t> Decompress(std::vector<uint8_t> const &input);

        /**
         * Compress data to QFS format
         * @param input Uncompressed data
         * @param inputSize Size of data
         * @param maxIterations Quality factor (higher = better compression, slower)
         * @return Compressed data
         */
        static std::vector<uint8_t> Compress(uint8_t const *input, size_t inputSize, int maxIterations = DEFAULT_MAX_ITERATIONS);
        static std::vector<uint8_t> Compress(std::vector<uint8_t> const &input, int maxIterations = DEFAULT_MAX_ITERATIONS);

      private:
        static constexpr uint8_t QFS_MAGIC_BYTE0 = 0x10;
        static constexpr uint8_t QFS_MAGIC_BYTE1 = 0xFB;

        static void CopyBytes(uint8_t *dest, uint8_t const *src, size_t len);
        static void CopyOverlapping(uint8_t *dest, uint8_t const *src, size_t len);
    };

} // namespace LibOpenNFS::Shared