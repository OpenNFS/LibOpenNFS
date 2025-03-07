#pragma once

#include <string>
#include <unordered_map>

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
    if (nfsVerString == "NFS_1")
        return NFSVersion::NFS_1;
    else if (nfsVerString == "NFS_2")
        return NFSVersion::NFS_2;
    else if (nfsVerString == "NFS_2_PS1")
        return NFSVersion::NFS_2_PS1;
    else if (nfsVerString == "NFS_2_SE")
        return NFSVersion::NFS_2_SE;
    else if (nfsVerString == "NFS_3")
        return NFSVersion::NFS_3;
    else if (nfsVerString == "NFS_3_PS1")
        return NFSVersion::NFS_3_PS1;
    else if (nfsVerString == "NFS_4")
        return NFSVersion::NFS_4;
    else if (nfsVerString == "NFS_4_PS1")
        return NFSVersion::NFS_4_PS1;
    else if (nfsVerString == "MCO")
        return NFSVersion::MCO;
    else if (nfsVerString == "NFS_5")
        return NFSVersion::NFS_5;
    else
        return NFSVersion::UNKNOWN;
}