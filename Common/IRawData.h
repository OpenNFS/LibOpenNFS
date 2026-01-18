#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "NFSVersion.h"
#include "Utils.h"

#ifdef _MSC_VER
#include <stdlib.h>
#define bswap_16(x) _byteswap_ushort(x)
#define bswap_32(x) _byteswap_ulong(x)
#else
#define bswap_16(x) __builtin_bswap16(x)
#define bswap_32(x) __builtin_bswap32(x)
#endif

template <typename T> [[nodiscard]] static bool safe_read(std::ifstream &ifstream, T &structure, size_t const size = sizeof(T)) {
    return ifstream.read((char *)&structure, (std::streamsize)size).gcount() == size;
}

template <typename T> [[nodiscard]] static bool safe_read(std::ifstream &ifstream, std::vector<T> &vec) {
    return ifstream.read((char *)vec.data(), (std::streamsize)vec.size() * sizeof(T)).gcount() == vec.size() * sizeof(T);
}

template <typename T, typename = std::enable_if_t<std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>>>
[[nodiscard]] static bool safe_read_bswap(std::ifstream &ifstream, T &x) {
    bool const success{safe_read(ifstream, x)};
    if (std::is_same_v<T, uint16_t>) {
        x = bswap_16(x);
    } else {
        x = bswap_32(x);
    }
    return success;
}

template <typename T, size_t N, typename = std::enable_if_t<std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>>>
[[nodiscard]] static bool safe_read_bswap(std::ifstream &ifstream, std::array<T, N> &arr) {
    bool const success{safe_read(ifstream, arr, N * sizeof(T))};
    for (auto &elem : arr) {
        if (std::is_same_v<T, uint16_t>) {
            elem = bswap_16(elem);
        } else {
            elem = bswap_32(elem);
        }
    }
    return success;
}

template <typename T> static void write(std::ofstream &ofstream, T &x) {
    ofstream.write((char *)&x, (std::streamsize)sizeof(T));
}

template <typename T, typename = std::enable_if_t<std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>>>
static void write_bswap(std::ofstream &ofstream, T &x) {
    if (std::is_same_v<T, uint16_t>) {
        x = bswap_16(x);
    } else {
        x = bswap_32(x);
    }
    ofstream.write((char *)&x, (std::streamsize)sizeof(T));
}

template <typename T, size_t N, typename = std::enable_if_t<std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>>>
static void write_bswap(std::ofstream &ofstream, std::array<T, N> &arr) {
    for (auto &elem : arr) {
        if (std::is_same_v<T, uint16_t>) {
            elem = bswap_16(elem);
        } else {
            elem = bswap_32(elem);
        }
    }
    ofstream.write((char *)arr.data(), (std::streamsize)N * sizeof(T));
}

static constexpr uint32_t ONFS_SIGNATURE = 0x15B001C0;
std::array<uint8_t, 6> const quadToTriVertNumbers = {0, 1, 2, 0, 2, 3};

class IRawData {
  public:
    virtual ~IRawData() = default;

  protected:
    virtual bool _SerializeIn(std::ifstream &ifstream) = 0;
    virtual void _SerializeOut(std::ofstream &ofstream) = 0;
};
