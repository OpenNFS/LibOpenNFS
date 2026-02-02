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
    id = "";
    for (int i = 0; i< ID_LENGTH; i++) {
        char character;
        onfs_check(safe_read(ifstream, character));
        id += std::tolower(character);
    }

    // Read flag count, I think that means values until the serial
    uint16_t flagCount = 0;
    onfs_check(safe_read(ifstream, flagCount));
    ASSERT(flagCount == EXPECTED_FLAG_COUNT, "Flag count should be 9, other values are not supported");

    uint16_t isBonusUint = 0;
    onfs_check(safe_read(ifstream, isBonusUint));
    isBonus = (bool) isBonusUint;

    uint16_t isAvailableToAiUint = 0;
    onfs_check(safe_read(ifstream, isAvailableToAiUint));
    isAvailableToAi = (bool) isAvailableToAiUint;

    onfs_check(safe_read(ifstream, vehicleClass));
    onfs_check(safe_read(ifstream, unknown1));

    uint16_t isDlcCarUint = 0;
    onfs_check(safe_read(ifstream, isDlcCarUint));
    isDlcCar = (bool) isDlcCarUint;

    uint16_t isPoliceUint = 0;
    onfs_check(safe_read(ifstream, isPoliceUint));
    isPolice = (bool) isPoliceUint;

    uint16_t seatPositionUint = 0;
    onfs_check(safe_read(ifstream, seatPositionUint));
    seatPosition = (SeatPosition) seatPositionUint;

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
                _convertToUtf8(manufacturer);
                break;
            case MODEL:
                std::getline(ifstream, model, '\0');
                _convertToUtf8(model);
                break;
            case CAR_NAME:
                std::getline(ifstream, carName, '\0');
                _convertToUtf8(carName);
                break;
            case PRICE:
                std::getline(ifstream, price, '\0');
                _convertToUtf8(price);
                break;
            case STATUS:
                std::getline(ifstream, status, '\0');
                _convertToUtf8(status);
                break;
            case WEIGHT:
                std::getline(ifstream, weight, '\0');
                _convertToUtf8(weight);
                break;
            case WEIGHT_DISTRIBUTION:
                std::getline(ifstream, weightDistribution, '\0');
                _convertToUtf8(weightDistribution);
                break;
            case LENGTH:
                std::getline(ifstream, length, '\0');
                _convertToUtf8(length);
                break;
            case WIDTH:
                std::getline(ifstream, width, '\0');
                _convertToUtf8(width);
                break;
            case HEIGHT:
                std::getline(ifstream, height, '\0');
                _convertToUtf8(height);
                break;
            case ENGINE:
                std::getline(ifstream, engine, '\0');
                _convertToUtf8(engine);
                break;
            case DISPLACEMENT:
                std::getline(ifstream, displacement, '\0');
                _convertToUtf8(displacement);
                break;
            case HORSE_POWER:
                std::getline(ifstream, horsePower, '\0');
                _convertToUtf8(horsePower);
                break;
            case TORQUE:
                std::getline(ifstream, torque, '\0');
                _convertToUtf8(torque);
                break;
            case MAXIMUM_RPM:
                std::getline(ifstream, maximumRpm, '\0');
                _convertToUtf8(maximumRpm);
                break;
            case BRAKES:
                std::getline(ifstream, brakes, '\0');
                _convertToUtf8(brakes);
                break;
            case TIRES:
                std::getline(ifstream, tires, '\0');
                _convertToUtf8(tires);
                break;
            case TOP_SPEED:
                std::getline(ifstream, topSpeedText, '\0');
                _convertToUtf8(topSpeedText);
                break;
            case ZERO_TO_SIXTY:
                std::getline(ifstream, zeroToSixty, '\0');
                _convertToUtf8(zeroToSixty);
                break;
            case ZERO_TO_ONE_HUNDRED:
                std::getline(ifstream, zeroToOneHundred, '\0');
                _convertToUtf8(zeroToOneHundred);
                break;
            case TRANSMISSION:
                std::getline(ifstream, transmission, '\0');
                _convertToUtf8(transmission);
                break;
            case GEARBOX:
                std::getline(ifstream, gearbox, '\0');
                _convertToUtf8(gearbox);
                break;
            default:
                LogWarning("Unhandled string %d in fedata!", i);
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
        _convertToUtf8(current_string);
        history.push_back(current_string);

        ifstream.seekg(current_offset, std::ios::beg);
    }

    for (int i = 0; i < COLOR_COUNT; i++) {
        onfs_check(safe_read(ifstream, current_string_offset));
        current_offset = ifstream.tellg();
        ifstream.seekg(current_string_offset, std::ios::beg);

        std::string current_string = "";
        std::getline(ifstream, current_string, '\0');
        _convertToUtf8(current_string);
        primaryColourNames.push_back(current_string);

        ifstream.seekg(current_offset, std::ios::beg);
    }

    return true;
}

void FedataFile::_SerializeOut(std::ofstream &ofstream) {
    ASSERT(false, "Fedata output serialization is not currently implemented");
}

void FedataFile::_convertToUtf8(std::string &string) {
    // Go backwards, so we can insert after the current character if that makes sense
    for (int i = string.size() - 1; i >= 0; i--) {
        uint8_t character = *reinterpret_cast<uint8_t*>(&string[i]);
        if (character & 128) {
            uint8_t byte1 = (character >> 5) & 3 | 0xC0;
            uint8_t byte2 = character & 0x3F | 128;
            string.replace(i, 1, {*reinterpret_cast<char*>(&byte1), *reinterpret_cast<char*>(&byte2)});
        }
    }
}