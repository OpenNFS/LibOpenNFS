#include "Track.h"

#include "Common/TextureUtils.h"

namespace LibOpenNFS {
    Track::Track(NFSVersion const _nfsVersion, std::string const &_name, std::string const &_basePath,
                 std::string const &_tag)
        : nfsVersion(_nfsVersion), name(_name), basePath(_basePath), tag(_tag) {
        if (tag.empty()) {
            tag = name;
        }
        texturePath = TextureUtils::GetTrackTexturePath(basePath, name, nfsVersion);
    }
} // namespace LibOpenNFS