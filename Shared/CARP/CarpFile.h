#pragma once

#include <vector>

#include "../../Common/IRawData.h"

namespace LibOpenNFS::Shared {
    enum CarpEntry : uint8_t {
        SERIAL_NUMBER = 0,
        CAR_CLASSIFICATION = 1,
        MASS = 2,
        NUMBER_OF_GEARS_MANUAL = 3,
        NUMBER_OF_GEARS_AUTOMATIC = 75,
        GEAR_SHIFT_DELAY = 4,
        SHIFT_BLIP_IN_RPM = 5,
        BRAKE_BLIP_IN_RPM = 6,
        VELOCITY_TO_RPM_RATIO_MANUAL = 7,
        VELOCITY_TO_RPM_RATIO_AUTOMATIC = 76,
        GEAR_RATIOS_MANUAL = 8,
        GEAR_RATIOS_AUTOMATIC = 77,
        GEAR_EFFICIENCY_MANUAL = 9,
        GEAR_EFFICIENCY_AUTOMATIC = 78,
        TORQUE_CURVE = 10,
        FINAL_GEAR_MANUAL = 11,
        FINAL_GEAR_AUTOMATIC = 79,
        ENGINE_MINIMUM_RPM = 12,
        ENGINE_REDLINE_IN_RPM = 13,
        MAXIMUM_VELOCITY_OF_CAR = 14,
        TOP_SPEED_CAP = 15,
        FRONT_DRIVE_RATIO = 16,
        USES_ANTILOCK_BRAKE_SYSTEM = 17,
        MAXIMUM_BRAKING_DECELERATION = 18,
        FRONT_BIAS_BRAKE_RATIO = 19,
        GAS_INCREASING_CURVE = 20,
        GAS_DECREASING_CURVE = 21,
        BRAKE_INCREASING_CURVE = 22,
        BRAKE_DECREASING_CURVE = 23,
        WHEEL_BASE = 24,
        FRONT_GRIP_BIAS = 25,
        POWER_STEERING = 26,
        MINIMUM_STEERING_ACCELERATION = 27,
        TURN_IN_RAMP = 28,
        TURN_OUT_RAMP = 29,
        LATERAL_ACCELERATION_GRIP_MULTIPLIER = 30,
        AERODYNAMIC_DOWNFORCE_MULTIPLIER = 31,
        GAS_OFF_FACTOR = 32,
        G_TRANSFER_FACTOR = 33,
        TURNING_CIRCLE_RADIUS = 34,
        TIRE_SPECS_FRONT = 35,
        TIRE_SPECS_REAR = 36,
        TIRE_WEAR = 37,
        SLIDE_MULTIPLIER = 38,
        SPIN_VELOCITY_CAP = 39,
        SLIDE_VELOCITY_CAP = 40,
        SLIDE_ASSISTANCE_FACTOR = 41,
        PUSH_FACTOR = 42,
        LOW_TURN_FACTOR = 43,
        HIGH_TURN_FACTOR = 44,
        PITCH_ROLL_FACTOR = 45,
        ROAD_BUMPINESS_FACTOR = 46,
        SPOILER_FUNCTION_TYPE = 47,
        SPOILER_ACTIVATION_SPEED = 48,
        GRADUAL_TURN_CUTOFF = 49,
        MEDIUM_TURN_CUTOFF = 50,
        SHARP_TURN_CUTOFF = 51,
        MEDIUM_TURN_SPEED_MODIFIER = 52,
        SHARP_TURN_SPEED_MODIFIER = 53,
        EXTREME_TURN_SPEED_MODIFIER = 54,
        SUBDIVIDE_LEVEL = 55,
        CAMERA_ARM = 56,
        BODY_DAMAGE = 57,
        ENGINE_DAMAGE = 58,
        SUSPENSION_DAMAGE = 59,
        ENGINE_TUNING = 60,
        BREAK_BALANCE = 61,
        STEERING_SPEED = 62,
        GEAR_RAT_FACTOR = 63,
        SUSPENSION_STIFFNESS = 64,
        AERO_FACTOR = 65,
        TIRE_FACTOR = 66,
        AI_ACC0_ACCELERATION_TABLE = 67,
        AI_ACC1_ACCELERATION_TABLE = 68,
        AI_ACC2_ACCELERATION_TABLE = 69,
        AI_ACC3_ACCELERATION_TABLE = 70,
        AI_ACC4_ACCELERATION_TABLE = 71,
        AI_ACC5_ACCELERATION_TABLE = 72,
        AI_ACC6_ACCELERATION_TABLE = 73,
        AI_ACC7_ACCELERATION_TABLE = 74,
        // NFS4 specific
        UNDERSTEER_GRADIENT = 80,
    };

    class CarpFile : IRawData {
      public:
        CarpFile() = default;
        static bool Load(std::string const &carpPath, CarpFile &carpFile);
        static void Save(std::string const &carpPath, CarpFile &carpFile);

        uint8_t serialNumber{0};
        uint8_t carClassification{0};
        float mass{1750.f};
        uint8_t numberOfGearsManual{6};
        uint8_t numberOfGearsAutomatic{6};
        uint8_t gearShiftDelay{6};
        std::vector<uint16_t> shiftBlipInRpm;
        std::vector<uint16_t> brakeBlipInRpm;
        std::vector<float> velocityToRpmRatioManual;
        std::vector<float> velocityToRpmRatioAutomatic;
        std::vector<float> gearRatiosManual;
        std::vector<float> gearRatiosAutomatic;
        std::vector<float> gearEfficiencyManual;
        std::vector<float> gearEfficiencyAutomatic;
        std::vector<float> torqueCurve;
        float finalGearManual{3.5f};
        float finalGearAutomatic{3.5f};
        uint32_t engineMinimumRpm{1000};
        uint32_t engineRedlineInRpm{7000};
        float maximumVelocityOfCar{80.f};
        float topSpeedCap{80.f};
        float frontDriveRatio{0.f};
        bool usesAntilockBrakeSystem{false};
        float maximumBrakingDeceleration{10.f};
        float frontBiasBrakeRatio{0.6f};
        std::vector<uint8_t> gasIncreasingCurve;
        std::vector<uint8_t> gasDecreasingCurve;
        std::vector<float> brakeIncreasingCurve;
        std::vector<float> brakeDecreasingCurve;
        float wheelBase{2.5f};
        float frontGripBias{0.5f};
        bool powerSteering{true};
        float minimumSteeringAcceleration{18.f};
        float turnInRamp{16.f};
        float turnOutRamp{32.f};
        float lateralAccelerationGripMultiplier{3.5f};
        float aerodynamicDownforceMultiplier{0.002f};
        float gasOffFactor{0.4f};
        float gTransferFactor{0.5f};
        float turningCircleRadius{11.f};
        std::vector<uint16_t> tireSpecsFront;
        std::vector<uint16_t> tireSpecsRear;
        float tireWear{0.f};
        float slideMultiplier{0.6f};
        float spinVelocityCap{0.4f};
        float slideVelocityCap{0.4f};
        float slideAssistanceFactor{150.f};
        uint32_t pushFactor{9000};
        float lowTurnFactor{0.024f};
        float highTurnFactor{0.03f};
        float pitchRollFactor{0.68f};
        float roadBumpinessFactor{0.59f};
        uint8_t spoilerFunctionType{1};
        float spoilerActivationSpeed{0.f};
        uint16_t gradualTurnCutoff{100};
        uint16_t mediumTurnCutoff{150};
        uint16_t sharpTurnCutoff{170};
        float mediumTurnSpeedModifier{0.98f};
        float sharpTurnSpeedModifier{0.95f};
        float extremeTurnSpeedModifier{0.9f};
        uint8_t subdivideLevel{3};
        float cameraArm{0.25f};
        float bodyDamage{0.f};
        float engineDamage{0.f};
        float suspensionDamage{0.f};
        float engineTuning{1.f};
        float breakBalance{0.f};
        float steeringSpeed{1.f};
        float gearRatFactor{1.f};
        float suspensionStiffness{1.f};
        float aeroFactor{1.f};
        float tireFactor{1.f};
        std::vector<float> aiAcc0AccelerationTable;
        std::vector<float> aiAcc1AccelerationTable;
        std::vector<float> aiAcc2AccelerationTable;
        std::vector<float> aiAcc3AccelerationTable;
        std::vector<float> aiAcc4AccelerationTable;
        std::vector<float> aiAcc5AccelerationTable;
        std::vector<float> aiAcc6AccelerationTable;
        std::vector<float> aiAcc7AccelerationTable;
        // NFS4 specific
        float understeerGradient{1.f};

      private:
        bool _SerializeIn(std::ifstream &ifstream) override;
        void _SerializeOut(std::ofstream &ofstream) override;
    };
} // namespace LibOpenNFS::Shared