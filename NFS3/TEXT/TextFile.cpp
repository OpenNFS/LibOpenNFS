#include "TextFile.h"

#include "Common/Logging.h"

using namespace LibOpenNFS::NFS3;

bool TextFile::Load(std::string const &textPath, TextFile &textFile) {
    std::ifstream text(textPath, std::ios::in | std::ios::binary);

    bool const loadStatus{textFile._SerializeIn(text)};
    text.close();

    return loadStatus;
}

void TextFile::Save(std::string const &textPath, TextFile &textFile) {
    std::ofstream text(textPath, std::ios::out | std::ios::binary);
    textFile._SerializeOut(text);
}

bool TextFile::_SerializeIn(std::ifstream &ifstream) {
    ifstream.seekg(TRACK_NAMES_OFFSET, std::ios::beg);
    uint32_t current_string_offset, current_offset;
    for (int i = 0; i < TRACK_COUNT; i++) {
        onfs_check(safe_read(ifstream, current_string_offset));
        current_offset = ifstream.tellg();
        ifstream.seekg(current_string_offset, std::ios::beg);

        std::string current_string = "";
        std::getline(ifstream, current_string, '\0');
        trackNames.push_back(current_string);

        ifstream.seekg(current_offset, std::ios::beg);
    }

    return true;
}

void TextFile::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "Text output serialization is not currently implemented");
}
