#include "FedataFile.h"

#include "Common/Logging.h"

using namespace LibOpenNFS::NFS3;

enum StringField {
        MANUFACTURER = 0,
        MODEL,
        CAR_NAME,
        PRICE,
        STATUS,
        WEIGHT,
        WEIGHT_DISTRIBUTION,
        LENGTH,
        WIDTH,
        HEIGHT,
        ENGINE,
        DISPLACEMENT,
        HORSE_POWER,
        TORQUE,
        MAXIMUM_RPM,
        BRAKES,
        TIRES,
        TOP_SPEED,
        ZERO_TO_SIXTY,
        ZERO_TO_ONE_HUNDRED,
        TRANSMISSION,
        GEARBOX
};

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
    // Go get the offset of car id
    ifstream.seekg(0, std::ios::beg);
    char carId[ID_LENGTH + 1];
    onfs_check(safe_read(ifstream, carId, sizeof(char) * ID_LENGTH));
    carId[ID_LENGTH] = '\0';
    id = carId;

    // Read flag count, I think that means values until the serial
    uint16_t flagCount = 0;
    onfs_check(safe_read(ifstream, flagCount));
    ASSERT(flagCount == EXPECTED_FLAG_COUNT, "Flag count should be 9, other values are not supported");

    u_int16_t isBonusUint = 0;
    onfs_check(safe_read(ifstream, isBonusUint));
    isBonus = (bool) isBonusUint;

    u_int16_t isAvailableToAiUint = 0;
    onfs_check(safe_read(ifstream, isAvailableToAiUint));
    isAvailableToAi = (bool) isAvailableToAiUint;

    onfs_check(safe_read(ifstream, vehicleClass));
    onfs_check(safe_read(ifstream, unknown1));

    u_int16_t isDlcCarUint = 0;
    onfs_check(safe_read(ifstream, isDlcCarUint));
    isDlcCar = (bool) isDlcCarUint;

    u_int16_t isPoliceUint = 0;
    onfs_check(safe_read(ifstream, isPoliceUint));
    isPolice = (bool) isPoliceUint;

    onfs_check(safe_read(ifstream, seat));
    onfs_check(safe_read(ifstream, unknown2));
    onfs_check(safe_read(ifstream, unknown3));
    onfs_check(safe_read(ifstream, serial));

    ifstream.seekg(STATS_OFFSET, std::ios::beg);
    onfs_check(safe_read(ifstream, acceleration));
    onfs_check(safe_read(ifstream, topSpeed));
    onfs_check(safe_read(ifstream, handling));
    onfs_check(safe_read(ifstream, breaking));
    onfs_check(safe_read(ifstream, unknownStat));

    uint8_t stringEntryCount = 0;
    onfs_check(safe_read(ifstream, stringEntryCount));
    ASSERT(stringEntryCount == EXPECTED_STRING_ENTRIES, "Only a string entry count of 40 is supported");

    // Start reading strings
    ifstream.seekg(STRING_OFFSET_OFFSET, std::ios::beg);
    uint32_t current_string_offset, current_offset;
    for (int i = 0; i < stringEntryCount - HISTORY_COUNT - COLOR_COUNT; i++) {
        onfs_check(safe_read(ifstream, current_string_offset));
        current_offset = ifstream.tellg();
        ifstream.seekg(current_string_offset, std::ios::beg);
        switch ((StringField) i) {
            case MANUFACTURER:
                std::getline(ifstream, manufacturer, '\0');
                break;
            case MODEL:
                std::getline(ifstream, model, '\0');
                break;
            case CAR_NAME:
                std::getline(ifstream, carName, '\0');
                break;
            case PRICE:
                std::getline(ifstream, price, '\0');
                break;
            case STATUS:
                std::getline(ifstream, status, '\0');
                break;
            case WEIGHT:
                std::getline(ifstream, weight, '\0');
                break;
            case WEIGHT_DISTRIBUTION:
                std::getline(ifstream, weightDistribution, '\0');
                break;
            case LENGTH:
                std::getline(ifstream, length, '\0');
                break;
            case WIDTH:
                std::getline(ifstream, width, '\0');
                break;
            case HEIGHT:
                std::getline(ifstream, height, '\0');
                break;
            case ENGINE:
                std::getline(ifstream, engine, '\0');
                break;
            case DISPLACEMENT:
                std::getline(ifstream, displacement, '\0');
                break;
            case HORSE_POWER:
                std::getline(ifstream, horsePower, '\0');
                break;
            case TORQUE:
                std::getline(ifstream, torque, '\0');
                break;
            case MAXIMUM_RPM:
                std::getline(ifstream, maximumRpm, '\0');
                break;
            case BRAKES:
                std::getline(ifstream, brakes, '\0');
                break;
            case TIRES:
                std::getline(ifstream, tires, '\0');
                break;
            case TOP_SPEED:
                std::getline(ifstream, topSpeedText, '\0');
                break;
            case ZERO_TO_SIXTY:
                std::getline(ifstream, zeroToSixty, '\0');
                break;
            case ZERO_TO_ONE_HUNDRED:
                std::getline(ifstream, zeroToOneHundred, '\0');
                break;
            case TRANSMISSION:
                std::getline(ifstream, transmission, '\0');
                break;
            case GEARBOX:
                std::getline(ifstream, gearbox, '\0');
                break;
            default:
                LogInfo("Unhandled string %d in fedata!", i);
                break;
        }
        ifstream.seekg(current_offset, std::ios::beg);
    }

    for (int i = 0; i < HISTORY_COUNT; i++) {
        onfs_check(safe_read(ifstream, current_string_offset));
        current_offset = ifstream.tellg();
        ifstream.seekg(current_string_offset, std::ios::beg);

        std::string current_string = "";
        std::getline(ifstream, current_string, '\0');
        history.push_back(current_string);

        ifstream.seekg(current_offset, std::ios::beg);
    }

    for (int i = 0; i < COLOR_COUNT; i++) {
        onfs_check(safe_read(ifstream, current_string_offset));
        current_offset = ifstream.tellg();
        ifstream.seekg(current_string_offset, std::ios::beg);

        std::string current_string = "";
        std::getline(ifstream, current_string, '\0');
        primaryColourNames.push_back(current_string);

        ifstream.seekg(current_offset, std::ios::beg);
    }

    return true;
}

void FedataFile::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "Fedata output serialization is not currently implemented");
}
