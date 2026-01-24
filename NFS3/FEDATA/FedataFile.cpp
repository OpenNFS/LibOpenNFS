#include "FedataFile.h"

#include "Common/Logging.h"

using namespace LibOpenNFS::NFS3;

bool FedataFile::Load(std::string const &fedataPath, FedataFile &fedataFile) {
    std::ifstream fedata(fedataPath, std::ios::in | std::ios::binary);

    bool const loadStatus{fedataFile._SerializeIn(fedata)};
    fedata.close();

    return loadStatus;
}

void FedataFile::Save(std::string const &fedataPath, FedataFile &fedataFile) {
    std::ofstream fedata(fedataPath, std::ios::out | std::ios::binary);
    fedataFile._SerializeOut(fedata);
}

bool FedataFile::_SerializeIn(std::ifstream &ifstream) {
    // TODO: Hugely incomplete. Old style parser shoehorned into new format, need all structs. No seekg. /AS
    // Go get the offset of car name
    ifstream.seekg(0, std::ios::beg);
    char carId[5] = "1234";  // Make sure the string ends with \0
    onfs_check(safe_read(ifstream, carId, sizeof(char) * 4));
    id = carId;

    uint32_t menuNameOffset = 0;
    ifstream.seekg(MENU_NAME_FILEPOS_OFFSET, std::ios::beg);
    onfs_check(safe_read(ifstream, menuNameOffset));
    ifstream.seekg(menuNameOffset, std::ios::beg);

    char carMenuName[64];
    onfs_check(safe_read(ifstream, carMenuName, sizeof(char) * 64));
    menuName = carMenuName;

    // Jump to location of FILEPOS table for car colour names
    ifstream.seekg(COLOUR_TABLE_OFFSET, std::ios::beg);
    // Read that table in
    uint32_t colourNameOffset;
    onfs_check(safe_read(ifstream, colourNameOffset));
    ifstream.seekg(colourNameOffset, std::ios::beg);
    for(std::string colourName; std::getline(ifstream, colourName, '\0'); ) {
        primaryColourNames.emplace_back(colourName.begin(), colourName.end());
        LogInfo("Adding color: %s", colourName.c_str());
    }

    return true;
}

void FedataFile::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "Fedata output serialization is not currently implemented");
}
