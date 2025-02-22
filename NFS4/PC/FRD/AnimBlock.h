#pragma once

#include "Common/IRawData.h"
#include "../Common.h"
#include "BaseObjectBlock.h"

namespace LibOpenNFS::NFS4 {
    class AnimBlock : public BaseObjectBlock {
      public:
        struct Keyframe {
            glm::ivec3 pt;
            glm::i16vec4 quat;
        };
        explicit AnimBlock(XObjHeader const &_header, std::ifstream &frd);
        bool _SerializeIn(std::ifstream &frd) override;
        void _SerializeOut(std::ofstream &frd) override;

        uint16_t unknown;
        uint8_t type;
        uint8_t id;
        uint16_t nKeyframes;
        uint16_t delay;
        std::vector<Keyframe> keyframes;
    };
} // namespace LibOpenNFS::NFS4
