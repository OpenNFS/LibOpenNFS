#pragma once

#include "../../Common/IRawData.h"
#include "../Common.h"
#include "BaseObjectBlock.h"

namespace LibOpenNFS::NFS4 {
    class XObjChunk : public IRawData {
      public:
        XObjChunk() = default;
        explicit XObjChunk(uint32_t _nObjects, std::ifstream &frd);
        void _SerializeOut(std::ofstream &frd) override;

        uint32_t nObjects{};
        std::vector<XObjHeader> objectHeaders;
        std::vector<std::unique_ptr<BaseObjectBlock>> objectBlocks;

      protected:
        bool _SerializeIn(std::ifstream &frd) override;
    };
} // namespace LibOpenNFS::NFS4
