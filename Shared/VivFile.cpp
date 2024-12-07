#include "VivFile.h"

#include <cstring>
#include <filesystem>
#include <sstream>

#include "Common/Logging.h"

namespace LibOpenNFS::Shared {
    bool VivFile::Load(std::string const &vivPath, VivFile &vivFile) {
        LogInfo("Loading VIV File located at %s", vivPath.c_str());
        std::ifstream viv(vivPath, std::ios::in | std::ios::binary);

        bool const loadStatus = vivFile._SerializeIn(viv);
        viv.close();

        return loadStatus;
    }

    void VivFile::Save(std::string const &vivPath, VivFile &vivFile) {
        LogInfo("Saving CAN File to %s", vivPath.c_str());
        std::ofstream viv(vivPath, std::ios::out | std::ios::binary);
        vivFile._SerializeOut(viv);
    }

    bool VivFile::Extract(std::string const &outPath, VivFile &vivFile) {
        std::filesystem::create_directories(outPath);

        for (uint32_t fileIdx = 0; fileIdx < vivFile.nFiles; ++fileIdx) {
            VivEntry &curFile{vivFile.files.at(fileIdx)};

            std::stringstream out_file_path;
            out_file_path << outPath << std::filesystem::path::preferred_separator << curFile.filename;

            std::ofstream out(out_file_path.str(), std::ios::out | std::ios::binary);
            if (!out.is_open()) {
                LogWarning("Error while creating output file %s", out_file_path.str().c_str());
                return false;
            }
            out.write((char *)curFile.data.data(), curFile.data.size());
            out.close();
        }
        return true;
    }

    bool VivFile::_SerializeIn(std::ifstream &ifstream) {
        onfs_check(safe_read(ifstream, vivHeader));

        if (memcmp(vivHeader, "BIGF", sizeof(vivHeader)) != 0) {
            LogWarning("Not a valid VIV file (BIGF header missing)");
            return false;
        }

        onfs_check(safe_read_bswap(ifstream, vivSize));

        onfs_check(safe_read_bswap(ifstream, nFiles));
        files.resize(nFiles);

        LogInfo("VIV contains %d files", nFiles);
        onfs_check(safe_read_bswap(ifstream, startPos));

        std::streampos currentPos = ifstream.tellg();

        for (uint8_t fileIdx = 0; fileIdx < nFiles; ++fileIdx) {
            ifstream.seekg(currentPos, std::ios_base::beg);
            uint32_t filePos = 0, fileSize = 0;
            onfs_check(safe_read_bswap(ifstream, filePos));
            onfs_check(safe_read_bswap(ifstream, fileSize));

            VivEntry &curFile{files.at(fileIdx)};
            int pos = 0;
            char c = ' ';
            ifstream.read(&c, sizeof(char));
            while (c != '\0') {
                curFile.filename[pos] = tolower(c);
                pos++;
                ifstream.read(&c, sizeof(char));
            }
            curFile.filename[pos] = '\0';

            currentPos = ifstream.tellg();
            ifstream.seekg(filePos, std::ios_base::beg);
            curFile.data.resize(fileSize);
            ifstream.read(reinterpret_cast<char *>(curFile.data.data()), fileSize);
            LogInfo("File %s was loaded successfully", curFile.filename);
        }

        return true;
    }

    void VivFile::_SerializeOut(std::ofstream &ofstream) {
        ASSERT(false, "VIV Output serialization is not implemented yet");
    }
} // namespace LibOpenNFS::Shared
