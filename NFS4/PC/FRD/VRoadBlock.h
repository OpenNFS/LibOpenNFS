#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS::NFS4 {
    class VRoadBlock : public IRawData {
      public:
        VRoadBlock() = default;
        explicit VRoadBlock(std::ifstream &frd);
        void _SerializeOut(std::ofstream &frd) override;

        glm::vec3 refPt;
        glm::vec3 normal, forward, right;
        float leftWall, rightWall;
        float unknown1[2];
        uint32_t unknown2[5];

      protected:
        bool _SerializeIn(std::ifstream &frd) override;
    };
} // namespace LibOpenNFS::NFS4
