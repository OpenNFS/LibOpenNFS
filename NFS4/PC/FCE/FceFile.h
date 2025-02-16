#pragma once

#include "../../Common/IRawData.h"

namespace LibOpenNFS::NFS4 {
    class FceFile final : IRawData {
      public:
        struct Colour {
            uint8_t H, S, B, T;
        };

        struct Triangle {
            uint32_t texPage;
            uint32_t vertex[3];  // Local indexes, add part first Vert index from "partFirstVertIndices"
            uint16_t padding[6]; // 00FF
            uint32_t polygonFlags;
            float uvTable[6]; // U1 U2 U3, V1 V2 V3
        };

        struct CarPart {
            std::vector<glm::vec3> vertices;
            std::vector<glm::vec3> normals;
            std::vector<Triangle> triangles;
        };

        // Valid values for components:
        //    K : "H" (Headlights); "T" (Taillights); "B" (Brakelight); "R" (Reverse light); "P" (Direction indicator); "S" (Siren);
        //    C : "W" (White); "R" (Red); "B" (Blue); "O" (Orange); "Y" (Yellow)
        //    B : "Y" (Yes); "N" (No)
        //    F : "O" (Flashing at moment 1); "E" (Flashing at moment 2); "N" (No flashing)
        //    I : Number between 0 and 9 with 0 being broken (normal max 5)
        //   Next only used with flashing lights:
        //    T : Number between 1 and 9 with 9 being longest time and 0 being constant (normal max 5)
        //    D : Number between 0 and 9 with 9 being longest delay and 0 no delay (normal max 2)
        struct DUMMY {
            char data[64];
            // char kind, colour, breakable, flashing, intensity, time, delay;
        };

        FceFile() = default;
        static bool Load(std::string const &fcePath, FceFile &fceFile);
        static void Save(std::string const &fcePath, FceFile &fceFile);

        // Raw file data
        uint32_t header; // Value always seems to be 14 10 10 00
        uint32_t unknown;
        uint32_t nTriangles;
        uint32_t nVertices;
        uint32_t nArts;
        uint32_t vertTblOffset;
        uint32_t normTblOffset;
        uint32_t triTblOffset;
        uint32_t tempStoreOffsets[3]; // -- ALL offset from 0x2038
        uint32_t undamagedVertsOffset;
        uint32_t undamagedNormsOffset;
        uint32_t damagedVertsOffset;
        uint32_t damagedNormsOffset;
        uint32_t unknownAreaOffset;
        uint32_t driverMovementOffset;
        uint32_t unknownOffsets[2];
        glm::vec3 modelHalfSize;
        uint32_t nDummies; // 0..16
        glm::vec3 dummyCoords[16];
        uint32_t nParts;
        glm::vec3 partCoords[64];
        uint32_t partFirstVertIndices[64];
        uint32_t partNumVertices[64];
        uint32_t partFirstTriIndices[64];
        uint32_t partNumTriangles[64];
        uint32_t nColours;
        Colour primaryColours[16];
        Colour interiorColours[16];
        Colour secondaryColours[16];
        Colour driverHairColours[16];
        uint8_t unknownTable[260]; // Probably part related, with 4 byte table header?
        DUMMY dummyObjectInfo[16];
        char partNames[64][64];
        uint8_t unknownTable2[528];
        std::vector<CarPart> carParts;

        // Derived
        bool isTraffic{};

      private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS::NFS4
