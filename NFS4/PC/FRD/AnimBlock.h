#pragma once

#include "../Common.h"
#include "BaseObjectBlock.h"
#include "Common/IRawData.h"
#include "Shared/AnimKeyframe.h"

namespace LibOpenNFS::NFS4 {
    class AnimBlock : public BaseObjectBlock {
      public:
        explicit AnimBlock(XObjHeader const &_header, std::ifstream &frd);
        bool _SerializeIn(std::ifstream &frd) override;
        void _SerializeOut(std::ofstream &frd) override;

        uint16_t unknown;
        uint8_t type;
        uint8_t id;
        uint16_t nKeyframes;
        uint16_t delay;
        std::vector<AnimKeyframe> keyframes;
    };
} // namespace LibOpenNFS::NFS4
