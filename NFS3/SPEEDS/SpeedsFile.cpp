#include "SpeedsFile.h"

#include "Common/Logging.h"

using namespace LibOpenNFS::NFS3;

bool SpeedsFile::Load(std::string const &speedBinPath, SpeedsFile &speedFile) {
    LogInfo("Loading FRD File located at %s", speedBinPath.c_str());
    std::ifstream speedBin(speedBinPath, std::ios::in | std::ios::binary);

    bool const loadStatus{speedFile._SerializeIn(speedBin)};
    speedBin.close();

    return loadStatus;
}

void SpeedsFile::Save(std::string const &speedBinPath, SpeedsFile &speedFile) {
    LogInfo("Saving SPEED BIN File to %s", speedBinPath.c_str());
    std::ofstream speedBin(speedBinPath, std::ios::out | std::ios::binary);
    speedFile._SerializeOut(speedBin);
}

bool SpeedsFile::_SerializeIn(std::ifstream &ifstream) {
    // Tactical grab of the file size
    ifstream.ignore(std::numeric_limits<std::streamsize>::max());
    m_uFileSize = ifstream.gcount();
    ifstream.clear();
    ifstream.seekg(0, std::ios_base::beg);

    speeds.resize(m_uFileSize);
    onfs_check(safe_read(ifstream, speeds));

    return true;
}

void SpeedsFile::_SerializeOut(std::ofstream &ofstream) {
    ofstream.write((char *)speeds.data(), m_uFileSize);
    ofstream.close();
}

void SpeedsFile::SaveCSV(std::string const &speedsCsvPath, SpeedsFile const &speedFile) {
    LogInfo("Saving SPEED BIN File to CSV: %s", speedsCsvPath.c_str());
    std::ofstream speedCsv(speedsCsvPath, std::ios::out | std::ios::binary);

    for (auto &speed : speedFile.speeds) {
        speedCsv << static_cast<uint16_t>(speed) << "," << std::endl;
    }

    speedCsv.close();
}
