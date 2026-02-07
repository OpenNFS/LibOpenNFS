#include "TextFile.h"

#include "Common/Logging.h"

using namespace LibOpenNFS::NFS4;

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
    uint32_t current_string_offset;
    for(uint32_t offset : TRACK_NAME_OFFSETS) {
        ifstream.seekg(offset, std::ios::beg);
        onfs_check(safe_read(ifstream, current_string_offset));
        ifstream.seekg(current_string_offset, std::ios::beg);

        std::string trackName = "";
        std::getline(ifstream, trackName, '\0');
        trackNames.emplace_back(trackName.begin(), trackName.end());
    }

    return true;
}

void TextFile::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "Text output serialization is not currently implemented");
}
