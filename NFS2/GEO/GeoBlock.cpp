#include "GeoBlock.h"

#include "Common/Logging.h"

namespace LibOpenNFS::NFS2 {
    template <typename Platform> GeoBlock<Platform>::GeoBlock(std::ifstream &ifstream, uint32_t const _partIdx) : partIdx(_partIdx) {
        ASSERT(this->GeoBlock::_SerializeIn(ifstream), "Failed to serialize GeoBlock from file stream");
    }

    template <> bool GeoBlock<PC>::_SerializeIn(std::ifstream &ifstream) {
        std::streamoff const headerStart = ifstream.tellg();

        onfs_check(safe_read(ifstream, header));
        onfs_check(header.padding[0] == 0 && header.padding[1] == 1 && header.padding[2] == 1);
        vertices.resize(header.nVerts);
        onfs_check(safe_read(ifstream, vertices));

        std::streamoff const headerEnd = ifstream.tellg();

        // Polygon Table start is aligned on 4 Byte boundary
        if (((headerStart - headerEnd) % 4)) {
            LogDebug("Part %u [%s] Polygon Table Pre-Pad Contents: ", partIdx, std::string(PC::PART_NAMES[partIdx]).c_str());
            std::vector<uint16_t> pad(3);
            onfs_check(safe_read(ifstream, pad));
            for (uint32_t i = 0; i < 3; ++i) {
                LogDebug("%u", pad[i]);
            }
        }

        polygons.resize(header.nPolygons);
        onfs_check(safe_read(ifstream, polygons));

        return true;
    }

    template <> bool GeoBlock<PS1>::_SerializeIn(std::ifstream &ifstream) {
        std::streamoff const headerStart = ifstream.tellg();

        onfs_check(safe_read(ifstream, header));
        onfs_check(header.padding[0] == 0 && header.padding[1] == 1 && header.padding[2] == 1);
        vertices.resize(header.nVerts);
        onfs_check(safe_read(ifstream, vertices));

        // If nVerts is ODD, we need to pad. Let's dump the contents of the pad though, in case there's data
        if (header.nVerts % 2) {
            LogDebug("Part %u [%s] Normal Table Pre-Pad Contents: ", partIdx, std::string(PC::PART_NAMES[partIdx]).c_str());
            std::vector<uint16_t> pad(3);
            onfs_check(safe_read(ifstream, pad));
            for (uint32_t i = 0; i < 3; ++i) {
                LogDebug("%u", pad[i]);
            }
        }

        normals.resize(header.nNormals);
        onfs_check(safe_read(ifstream, normals));

        // TODO: This is probably all wrong
        // Is this really a block type?
        switch (header.unknown1) {
        case 1:
            onfs_check(safe_read(ifstream, xblock_1));
            break;
        case 2:
            onfs_check(safe_read(ifstream, xblock_2));
            break;
        case 3:
            onfs_check(safe_read(ifstream, xblock_3));
            break;
        case 4:
            onfs_check(safe_read(ifstream, xblock_4));
            break;
        case 5:
            onfs_check(safe_read(ifstream, xblock_5));
            break;
        default:
            LogDebug("Unknown block type: %u", header.unknown1);
        }

        std::streamoff const end = ifstream.tellg();

        // Polygon Table start is aligned on 4 Byte boundary
        if (((headerStart - end) % 4)) {
            LogDebug("Part %u [%s] Polygon Table Pre-Pad Contents: ", partIdx, std::string(PC::PART_NAMES[partIdx]).c_str());
            std::vector<uint16_t> pad(3);
            onfs_check(safe_read(ifstream, pad));
            for (uint32_t i = 0; i < 3; ++i) {
                LogDebug("%u", pad[i]);
            }
        }

        polygons.resize(header.nPolygons);
        onfs_check(safe_read(ifstream, polygons));

        return true;
    }

    template <typename Platform> void GeoBlock<Platform>::_SerializeOut(std::ofstream &ofstream) {
        ASSERT(false, "GEO output serialization is not currently implemented");
    }

    template class GeoBlock<PS1>;
    template class GeoBlock<PC>;
} // namespace LibOpenNFS::NFS2