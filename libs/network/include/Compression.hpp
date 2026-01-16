/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Network compression utilities using LZ4
*/

#pragma once

#include <vector>
#include <cstdint>
#include <lz4.h>

namespace network {

    class Compression {
    public:
        static std::vector<uint8_t> CompressLZ4(const uint8_t* data, size_t size) {
            if (size == 0) return {};

            int maxCompressedSize = LZ4_compressBound(static_cast<int>(size));
            std::vector<uint8_t> compressed(maxCompressedSize);

            int compressedSize = LZ4_compress_default(
                reinterpret_cast<const char*>(data),
                reinterpret_cast<char*>(compressed.data()),
                static_cast<int>(size),
                maxCompressedSize
            );

            if (compressedSize <= 0) {
                return {};
            }

            compressed.resize(compressedSize);
            return compressed;
        }

        static std::vector<uint8_t> DecompressLZ4(const uint8_t* compressedData, size_t compressedSize, size_t originalSize) {
            if (compressedSize == 0 || originalSize == 0) return {};

            std::vector<uint8_t> decompressed(originalSize);

            int decompressedSize = LZ4_decompress_safe(
                reinterpret_cast<const char*>(compressedData),
                reinterpret_cast<char*>(decompressed.data()),
                static_cast<int>(compressedSize),
                static_cast<int>(originalSize)
            );

            if (decompressedSize < 0 || static_cast<size_t>(decompressedSize) != originalSize) {
                return {};
            }

            return decompressed;
        }

        static bool ShouldCompress(size_t originalSize, size_t compressedSize) {
            return compressedSize < originalSize &&
                   (originalSize - compressedSize) >= 20 &&
                   (static_cast<float>(compressedSize) / static_cast<float>(originalSize)) < 0.9f;
        }

        static constexpr size_t MIN_COMPRESSION_SIZE = 100;
    };

}
