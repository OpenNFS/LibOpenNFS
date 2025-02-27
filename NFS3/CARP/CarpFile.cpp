#include "CarpFile.h"

#include "Common/Logging.h"
#include "Common/TextureUtils.h"

namespace LibOpenNFS::NFS3 {
    bool CarpFile::Load(std::string const &carpPath, CarpFile &carpFile) {
        LogInfo("Loading carp.txt File located at %s", carpPath.c_str());
        std::ifstream carp(carpPath, std::ios::in | std::ios::binary);

        bool const loadStatus{carpFile._SerializeIn(carp)};
        carp.close();

        return loadStatus;
    }

    void CarpFile::Save(std::string const &carpPath, CarpFile &carpFile) {
        LogInfo("Saving carp.txt File to %s", carpPath.c_str());
        std::ofstream carp(carpPath, std::ios::out | std::ios::binary);
        carpFile._SerializeOut(carp);
    }

    bool CarpFile::_SerializeIn(std::ifstream &ifstream) {       
        std::string str, data;

        while (std::getline(ifstream, str)) {
            CarpEntry entry = (CarpEntry) std::atoi(str.substr(str.rfind('(') + 1, str.rfind(')') -1 ).c_str());  // The entry number is between () in the string
            std::getline(ifstream, data);
            data = data.substr(0, data.rfind("\n"));
            data = data.substr(0, data.rfind("\r"));
            switch (entry)
            {
                case CarpEntry::SERIAL_NUMBER:
                    if (data.empty()) {
                        return false;
                    }
                    serialNumber = (uint8_t) std::atoi(data.c_str());
                    break;
                case CarpEntry::MASS:
                    mass = std::atof(data.c_str());
                    break;
                case CarpEntry::CAR_CLASSIFICATION:
                    carClassification = (uint8_t) std::atoi(data.c_str());
                    break;
                case CarpEntry::NUMBER_OF_GEARS_MANUAL:
                    numberOfGearsManual = (uint8_t) std::atoi(data.c_str());
                    break;
                case CarpEntry::NUMBER_OF_GEARS_AUTOMATIC:
                    numberOfGearsAutomatic = (uint8_t) std::atoi(data.c_str());
                    break;
                case CarpEntry::GEAR_SHIFT_DELAY:
                    gearShiftDelay = (uint8_t) std::atoi(data.c_str());
                    break;
                case CarpEntry::SHIFT_BLIP_IN_RPM:
                case CarpEntry::BRAKE_BLIP_IN_RPM:
                case CarpEntry::VELOCITY_TO_RPM_RATIO_MANUAL:
                case CarpEntry::VELOCITY_TO_RPM_RATIO_AUTOMATIC:
                case CarpEntry::GEAR_RATIOS_MANUAL:
                case CarpEntry::GEAR_RATIOS_AUTOMATIC:
                case CarpEntry::GEAR_EFFICIENCY_MANUAL:
                case CarpEntry::GEAR_EFFICIENCY_AUTOMATIC:
                case CarpEntry::TORQUE_CURVE:
                    LogWarning("Implementation for loading %i from carp.txt is missing", (int) entry);
                    break;
                case CarpEntry::FINAL_GEAR_MANUAL:
                    finalGearManual = std::atof(data.c_str());
                    break;
                case CarpEntry::FINAL_GEAR_AUTOMATIC:
                    finalGearAutomatic = std::atof(data.c_str());
                    break;
                case CarpEntry::ENGINE_MINIMUM_RPM:
                    engineMinimumRpm = (uint32_t) std::atoi(data.c_str());
                    break;
                case CarpEntry::ENGINE_REDLINE_IN_RPM:
                    engineRedlineInRpm = (uint32_t) std::atoi(data.c_str());
                    break;
                case CarpEntry::MAXIMUM_VELOCITY_OF_CAR:
                    maximumVelocityOfCar = std::atof(data.c_str());
                    break;
                case CarpEntry::TOP_SPEED_CAP:
                    topSpeedCap = std::atof(data.c_str());
                    break;
                case CarpEntry::FRONT_DRIVE_RATIO:
                    frontDriveRatio = std::atof(data.c_str());
                    break;
                case CarpEntry::USES_ANTILOCK_BRAKE_SYSTEM:
                    usesAntilockBrakeSystem = (data == "1");
                    break;
                case CarpEntry::MAXIMUM_BRAKING_DECELERATION:
                    maximumBrakingDeceleration  = std::atof(data.c_str());
                    break;
                case CarpEntry::FRONT_BIAS_BRAKE_RATIO:
                    frontBiasBrakeRatio  = std::atof(data.c_str());
                    break;
                case CarpEntry::GAS_INCREASING_CURVE:
                case CarpEntry::GAS_DECREASING_CURVE:
                case CarpEntry::BRAKE_INCREASING_CURVE:
                case CarpEntry::BRAKE_DECREASING_CURVE:
                    LogWarning("Implementation for loading %i from carp.txt is missing", (int) entry);
                    break;
                case CarpEntry::WHEEL_BASE:
                    wheelBase = std::atof(data.c_str());
                    break;
                case CarpEntry::FRONT_GRIP_BIAS:
                    frontGripBias = std::atof(data.c_str());
                    break;
                case CarpEntry::POWER_STEERING:
                    powerSteering = (data == "1");
                    break;
                case CarpEntry::MINIMUM_STEERING_ACCELERATION:
                    minimumSteeringAcceleration = std::atof(data.c_str());
                    break;
                case CarpEntry::TURN_IN_RAMP:
                    turnInRamp = std::atof(data.c_str());
                    break;
                case CarpEntry::TURN_OUT_RAMP:
                    turnOutRamp = std::atof(data.c_str());
                    break;
                case CarpEntry::LATERAL_ACCELERATION_GRIP_MULTIPLIER:
                    lateralAccelerationGripMultiplier = std::atof(data.c_str());
                    break;
                case CarpEntry::AERODYNAMIC_DOWNFORCE_MULTIPLIER:
                    aerodynamicDownforceMultiplier = std::atof(data.c_str());
                    break;
                case CarpEntry::GAS_OFF_FACTOR:
                    gasOffFactor = std::atof(data.c_str());
                    break;
                case CarpEntry::G_TRANSFER_FACTOR:
                    gTransferFactor = std::atof(data.c_str());
                    break;
                case CarpEntry::TURNING_CIRCLE_RADIUS:
                    turningCircleRadius = std::atof(data.c_str());
                    break;
                case CarpEntry::TIRE_SPECS_FRONT:
                    LogWarning("Implementation for loading %i from carp.txt is missing", (int) entry);
                    break;
                case CarpEntry::TIRE_SPECS_REAR:
                    LogWarning("Implementation for loading %i from carp.txt is missing", (int) entry);
                    break;
                case CarpEntry::TIRE_WEAR:
                    tireWear = std::atof(data.c_str());
                    break;
                case CarpEntry::SLIDE_MULTIPLIER:
                    slideMultiplier = std::atof(data.c_str());
                    break;
                case CarpEntry::SPIN_VELOCITY_CAP:
                    spinVelocityCap = std::atof(data.c_str());
                    break;
                case CarpEntry::SLIDE_VELOCITY_CAP:
                    slideVelocityCap = std::atof(data.c_str());
                    break;
                case CarpEntry::SLIDE_ASSISTANCE_FACTOR:
                    slideAssistanceFactor = std::atof(data.c_str());
                    break;
                case CarpEntry::PUSH_FACTOR:
                    pushFactor = (uint32_t) std::atoi(data.c_str());
                    break;
                case CarpEntry::LOW_TURN_FACTOR:
                    lowTurnFactor = std::atof(data.c_str());
                    break;
                case CarpEntry::HIGH_TURN_FACTOR:
                    highTurnFactor = std::atof(data.c_str());
                    break;
                case CarpEntry::PITCH_ROLL_FACTOR:
                    pitchRollFactor = std::atof(data.c_str());
                    break;
                case CarpEntry::ROAD_BUMPINESS_FACTOR:
                    roadBumpinessFactor = std::atof(data.c_str());
                    break;
                case CarpEntry::SPOILER_FUNCTION_TYPE:
                    spoilerFunctionType = (uint8_t) std::atoi(data.c_str());
                    break;
                case CarpEntry::SPOILER_ACTIVATION_SPEED:
                    spoilerActivationSpeed = std::atof(data.c_str());
                    break;
                case CarpEntry::GRADUAL_TURN_CUTOFF:
                    gradualTurnCutoff = (uint16_t) std::atoi(data.c_str());
                    break;
                case CarpEntry::MEDIUM_TURN_CUTOFF:
                    mediumTurnCutoff = (uint16_t) std::atoi(data.c_str());
                    break;
                case CarpEntry::SHARP_TURN_CUTOFF:
                    sharpTurnCutoff = (uint16_t) std::atoi(data.c_str());
                    break;
                case CarpEntry::MEDIUM_TURN_SPEED_MODIFIER:
                    mediumTurnSpeedModifier = std::atof(data.c_str());
                    break;
                case CarpEntry::SHARP_TURN_SPEED_MODIFIER:
                    sharpTurnSpeedModifier = std::atof(data.c_str());
                    break;
                case CarpEntry::EXTREME_TURN_SPEED_MODIFIER:
                    extremeTurnSpeedModifier = std::atof(data.c_str());
                    break;
                case CarpEntry::SUBDIVIDE_LEVEL:
                    subdivideLevel = (uint8_t) std::atoi(data.c_str());
                    break;
                case CarpEntry::CAMERA_ARM:
                    cameraArm = std::atof(data.c_str());
                    break;
                case CarpEntry::BODY_DAMAGE:
                    bodyDamage = std::atof(data.c_str());
                    break;
                case CarpEntry::ENGINE_DAMAGE:
                    engineDamage = std::atof(data.c_str());
                    break;
                case CarpEntry::SUSPENSION_DAMAGE:
                    suspensionDamage = std::atof(data.c_str());
                    break;
                case CarpEntry::ENGINE_TUNING:
                    engineTuning = std::atof(data.c_str());
                    break;
                case CarpEntry::BREAK_BALANCE:
                    breakBalance = std::atof(data.c_str());
                    break;
                case CarpEntry::STEERING_SPEED:
                    steeringSpeed = std::atof(data.c_str());
                    break;
                case CarpEntry::GEAR_RAT_FACTOR:
                    gearRatFactor = std::atof(data.c_str());
                    break;
                case CarpEntry::SUSPENSION_STIFFNESS:
                    suspensionStiffness = std::atof(data.c_str());
                    break;
                case CarpEntry::AERO_FACTOR:
                    aeroFactor = std::atof(data.c_str());
                    break;
                case CarpEntry::TIRE_FACTOR:
                    tireFactor = std::atof(data.c_str());
                    break;
                case CarpEntry::AI_ACC0_ACCELERATION_TABLE:
                case CarpEntry::AI_ACC1_ACCELERATION_TABLE:
                case CarpEntry::AI_ACC2_ACCELERATION_TABLE:
                case CarpEntry::AI_ACC3_ACCELERATION_TABLE:
                case CarpEntry::AI_ACC4_ACCELERATION_TABLE:
                case CarpEntry::AI_ACC5_ACCELERATION_TABLE:
                case CarpEntry::AI_ACC6_ACCELERATION_TABLE:
                case CarpEntry::AI_ACC7_ACCELERATION_TABLE:
                    LogWarning("Implementation for loading %i from carp.txt is missing", (int) entry);
                    break;
                default:
                    LogWarning("Could not process %i", (int) entry);
                    break;
            }
        }

        return true;
    }

    void CarpFile::_SerializeOut(std::ofstream &ofstream) {
        ASSERT(false, "HRZ Output serialization is not implemented yet");
    }
} // namespace LibOpenNFS::Shared
