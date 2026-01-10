#pragma once

#include "../../Common/IRawData.h"

#include "TrkBlock.h"
#include "TrkBlockHeader.h"
#include "VRoadBlock.h"

namespace LibOpenNFS::NFS4 {
    static constexpr uint8_t HEADER_LENGTH{28};

    class FrdFile : IRawData {
      public:
        FrdFile() = default;
        static bool Load(std::string const &frdPath, FrdFile &frdFile);
        static void Save(std::string const &frdPath, FrdFile &frdFile);

        // Raw File data
        char header[HEADER_LENGTH];
        uint32_t nBlocks;
        uint32_t numVRoad;
        uint32_t nTextures;
        NFSVersion version;
        std::vector<VRoadBlock> vroadBlocks;
        std::vector<TrkBlockHeader> trackBlockHeaders;
        std::vector<TrkBlock> trackBlocks;
        uint32_t nGlobalObjects[2];
        std::vector<XObjChunk> globalObjects;

      private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS::NFS4
