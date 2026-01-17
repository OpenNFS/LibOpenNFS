#include "QfsCompression.h"

namespace LibOpenNFS::Shared {
    bool QfsCompression::IsCompressed(uint8_t const *data, size_t const size) {
        if (size < 5)
            return false;
        return ((data[0] & 0xFE) == QFS_MAGIC_BYTE0) && (data[1] == QFS_MAGIC_BYTE1);
    }

    bool QfsCompression::IsCompressed(std::vector<uint8_t> const &data) {
        return IsCompressed(data.data(), data.size());
    }

    size_t QfsCompression::GetUncompressedSize(uint8_t const *data) {
        return (static_cast<size_t>(data[2]) << 16) | (static_cast<size_t>(data[3]) << 8) | static_cast<size_t>(data[4]);
    }

    std::vector<uint8_t> QfsCompression::Decompress(uint8_t const *input, size_t const inputSize) {
        if (!IsCompressed(input, inputSize)) {
            throw std::runtime_error("Data is not QFS compressed");
        }

        size_t const outputSize = GetUncompressedSize(input);
        std::vector<uint8_t> output(outputSize);

        // Skip header (5 or 8 bytes depending on format)
        size_t inPos = (input[0] & 0x01) ? 8 : 5;
        size_t outPos = 0;

        // Main decompression loop
        while (inPos < inputSize && input[inPos] < 0xFC) {
            uint8_t const packCode = input[inPos];
            uint8_t const byte1 = input[inPos + 1];
            uint8_t const byte2 = input[inPos + 2];

            if (!(packCode & 0x80)) {
                // 2-byte command
                size_t const literalLen = packCode & 0x03;
                CopyBytes(output.data() + outPos, input + inPos + 2, literalLen);
                inPos += literalLen + 2;
                outPos += literalLen;

                size_t const copyLen = ((packCode & 0x1C) >> 2) + 3;
                size_t const offset = ((packCode >> 5) << 8) + byte1 + 1;
                CopyOverlapping(output.data() + outPos, output.data() + outPos - offset, copyLen);
                outPos += copyLen;
            } else if (!(packCode & 0x40)) {
                // 3-byte command
                size_t const literalLen = (byte1 >> 6) & 0x03;
                CopyBytes(output.data() + outPos, input + inPos + 3, literalLen);
                inPos += literalLen + 3;
                outPos += literalLen;

                size_t const copyLen = (packCode & 0x3F) + 4;
                size_t const offset = (byte1 & 0x3F) * 256 + byte2 + 1;
                CopyOverlapping(output.data() + outPos, output.data() + outPos - offset, copyLen);
                outPos += copyLen;
            } else if (!(packCode & 0x20)) {
                // 4-byte command
                uint8_t const byte3 = input[inPos + 3];
                size_t const literalLen = packCode & 0x03;
                CopyBytes(output.data() + outPos, input + inPos + 4, literalLen);
                inPos += literalLen + 4;
                outPos += literalLen;

                size_t const copyLen = ((packCode >> 2) & 0x03) * 256 + byte3 + 5;
                size_t const offset = ((packCode & 0x10) << 12) + 256 * byte1 + byte2 + 1;
                CopyOverlapping(output.data() + outPos, output.data() + outPos - offset, copyLen);
                outPos += copyLen;
            } else {
                // Literal block
                size_t const literalLen = (packCode & 0x1F) * 4 + 4;
                CopyBytes(output.data() + outPos, input + inPos + 1, literalLen);
                inPos += literalLen + 1;
                outPos += literalLen;
            }
        }

        // Handle trailing bytes
        if (inPos < inputSize && outPos < outputSize) {
            size_t const trailingLen = input[inPos] & 0x03;
            CopyBytes(output.data() + outPos, input + inPos + 1, trailingLen);
            outPos += trailingLen;
        }

        return output;
    }

    std::vector<uint8_t> QfsCompression::Decompress(std::vector<uint8_t> const &input) {
        return Decompress(input.data(), input.size());
    }

    std::vector<uint8_t> QfsCompression::Compress(uint8_t const *input, size_t const inputSize, int const maxIterations) {
        constexpr size_t WINDOW_LEN = 1 << 17;
        constexpr size_t WINDOW_MASK = WINDOW_LEN - 1;

        // Allocate output buffer (worst case is slightly larger than input)
        std::vector<uint8_t> output(inputSize + 1024);

        // Build hash tables for fast string matching
        std::vector<int> revSimilar(WINDOW_LEN, -1);
        std::vector<std::vector<int>> revLast(256, std::vector<int>(256, -1));

        // Write header
        output[0] = 0x10;
        output[1] = 0xFB;
        output[2] = static_cast<uint8_t>(inputSize >> 16);
        output[3] = static_cast<uint8_t>((inputSize >> 8) & 0xFF);
        output[4] = static_cast<uint8_t>(inputSize & 0xFF);

        size_t outPos = 5;
        size_t lastWritten = 0;

        // Main compression loop
        for (size_t inPos = 0; inPos < inputSize; ++inPos) {
            // Update hash tables
            int *hashEntry = &revLast[input[inPos]][inPos + 1 < inputSize ? input[inPos + 1] : 0];
            int const prevOccurrence = revSimilar[inPos & WINDOW_MASK] = *hashEntry;
            *hashEntry = static_cast<int>(inPos);

            // Skip if already covered by a previous match
            if (inPos < lastWritten)
                continue;

            // Find best match
            size_t bestLen = 0;
            size_t bestOffset = 0;
            int iterations = 0;

            int searchPos = prevOccurrence;
            while (searchPos >= 0 && inPos - searchPos < WINDOW_LEN && iterations++ < maxIterations) {
                size_t len = 2;
                while (inPos + len < inputSize && len < 1028 && input[inPos + len] == input[searchPos + len]) {
                    ++len;
                }

                if (len > bestLen) {
                    bestLen = len;
                    bestOffset = inPos - searchPos;
                }

                searchPos = revSimilar[searchPos & WINDOW_MASK];
            }

            // Check if match is worthwhile
            if (bestLen > inputSize - inPos)
                bestLen = inputSize - inPos;
            if (bestLen <= 2)
                bestLen = 0;
            if (bestLen == 3 && bestOffset > 1024)
                bestLen = 0;
            if (bestLen == 4 && bestOffset > 16384)
                bestLen = 0;

            // Output compressed data
            if (bestLen > 0) {
                // First, flush any pending literal data
                while (inPos - lastWritten >= 4) {
                    size_t literalBlocks = (inPos - lastWritten) / 4 - 1;
                    if (literalBlocks > 0x1B)
                        literalBlocks = 0x1B;
                    output[outPos++] = static_cast<uint8_t>(0xE0 + literalBlocks);
                    size_t const literalLen = literalBlocks * 4 + 4;
                    std::memcpy(output.data() + outPos, input + lastWritten, literalLen);
                    lastWritten += literalLen;
                    outPos += literalLen;
                }

                size_t const pendingLiterals = inPos - lastWritten;

                // Encode match
                if (bestLen <= 10 && bestOffset <= 1024) {
                    // 2-byte encoding
                    output[outPos++] = static_cast<uint8_t>((((bestOffset - 1) >> 8) << 5) + ((bestLen - 3) << 2) + pendingLiterals);
                    output[outPos++] = static_cast<uint8_t>((bestOffset - 1) & 0xFF);
                    std::memcpy(output.data() + outPos, input + lastWritten, pendingLiterals);
                    outPos += pendingLiterals;
                    lastWritten = inPos + bestLen;
                } else if (bestLen <= 67 && bestOffset <= 16384) {
                    // 3-byte encoding
                    output[outPos++] = static_cast<uint8_t>(0x80 + (bestLen - 4));
                    output[outPos++] = static_cast<uint8_t>((pendingLiterals << 6) + ((bestOffset - 1) >> 8));
                    output[outPos++] = static_cast<uint8_t>((bestOffset - 1) & 0xFF);
                    std::memcpy(output.data() + outPos, input + lastWritten, pendingLiterals);
                    outPos += pendingLiterals;
                    lastWritten = inPos + bestLen;
                } else if (bestLen <= 1028 && bestOffset < WINDOW_LEN) {
                    // 4-byte encoding
                    size_t const adjustedOffset = bestOffset - 1;
                    output[outPos++] =
                        static_cast<uint8_t>(0xC0 + ((adjustedOffset >> 16) << 4) + (((bestLen - 5) >> 8) << 2) + pendingLiterals);
                    output[outPos++] = static_cast<uint8_t>((adjustedOffset >> 8) & 0xFF);
                    output[outPos++] = static_cast<uint8_t>(adjustedOffset & 0xFF);
                    output[outPos++] = static_cast<uint8_t>((bestLen - 5) & 0xFF);
                    std::memcpy(output.data() + outPos, input + lastWritten, pendingLiterals);
                    outPos += pendingLiterals;
                    lastWritten = inPos + bestLen;
                }
            }
        }

        // Flush remaining literal data
        while (inputSize - lastWritten >= 4) {
            size_t literalBlocks = (inputSize - lastWritten) / 4 - 1;
            if (literalBlocks > 0x1B)
                literalBlocks = 0x1B;
            output[outPos++] = static_cast<uint8_t>(0xE0 + literalBlocks);
            size_t const literalLen = literalBlocks * 4 + 4;
            std::memcpy(output.data() + outPos, input + lastWritten, literalLen);
            lastWritten += literalLen;
            outPos += literalLen;
        }

        // End marker with trailing bytes
        size_t const trailing = inputSize - lastWritten;
        output[outPos++] = static_cast<uint8_t>(0xFC + trailing);
        std::memcpy(output.data() + outPos, input + lastWritten, trailing);
        outPos += trailing;

        output.resize(outPos);
        return output;
    }

    std::vector<uint8_t> QfsCompression::Compress(std::vector<uint8_t> const &input, int const maxIterations) {
        return Compress(input.data(), input.size(), maxIterations);
    }

    void QfsCompression::CopyBytes(uint8_t *dest, uint8_t const *src, size_t const len) {
        std::memcpy(dest, src, len);
    }

    void QfsCompression::CopyOverlapping(uint8_t *dest, uint8_t const *src, size_t len) {
        while (len-- > 0) {
            *dest++ = *src++;
        }
    }

} // namespace LibOpenNFS::Shared