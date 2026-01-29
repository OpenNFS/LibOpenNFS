#pragma once

#include <string>

#include "../../lib/magic_enum/magic_enum.hpp"

enum class NFSVersion {
    UNKNOWN,
    NFS_1,
    NFS_2,
    NFS_2_PS1,
    NFS_2_SE,
    NFS_3,
    NFS_3_PS1,
    NFS_4,
    NFS_4_PS1,
    MCO,
    NFS_5
};

inline NFSVersion get_enum(const std::string& nfsVerString) {
    auto const nfsVersion = magic_enum::enum_cast<NFSVersion>(nfsVerString);
    if (nfsVersion.has_value()) {
        return nfsVersion.value();
    }

    return NFSVersion::UNKNOWN;
}