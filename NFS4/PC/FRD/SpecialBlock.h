#pragma once

#include "Common/IRawData.h"
#include "../Common.h"
#include "BaseObjectBlock.h"

namespace LibOpenNFS::NFS4 {
    class SpecialBlock : public BaseObjectBlock {
      public:
        explicit SpecialBlock(XObjHeader const &_header, std::ifstream &frd);
        bool _SerializeIn(std::ifstream &frd) override;
        void _SerializeOut(std::ofstream &frd) override;

        glm::vec3 location;
        float mass;
        float transform[9];
        glm::vec3 collisionDimensions;
        uint32_t unknown3;
        uint16_t unknown4;
        uint16_t unknown5;
    };
} // namespace LibOpenNFS::NFS4
