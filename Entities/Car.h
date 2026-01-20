#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "CarGeometry.h"
#include "Common/NFSVersion.h"

namespace LibOpenNFS {
    class Car {
      public:
        class Dummy {
          public:
            std::string name;
            glm::vec3 position;
            Dummy(char const *dummyName, glm::vec3 const position) {
                this->name = std::string(dummyName);
                this->position = position;
            }
        };

        class Colour {
          public:
            std::string colourName;
            glm::vec4 colour;
            glm::vec4 colourSecondary;
            Colour(std::string const &colourName, glm::vec4 const colour, glm::vec4 const colourSecondary = {0.0, 0.0, 0.0, 0.0}) {
                this->colourName = colourName;
                this->colour = colour;
                this->colourSecondary = colourSecondary;
            }
        };

        class MetaData {
          public:
            explicit MetaData() = default;
            std::string name = "Unset";
            std::vector<Dummy> dummies;
            std::vector<Colour> colours;
            std::vector<CarGeometry> meshes;
        };

        class PhysicsData {
          public:
            explicit PhysicsData() = default;
            // Old Fields
            float mass = 1750.f;

            // Engine (old)
            float maxEngineForce = 3000.f;   // Max engine force to apply
            float maxBreakingForce = 1000.f; // Max breaking force
            float maxSpeed = 100.f;          // Max speed before stop applying engine force

            // Steering (old)
            bool absoluteSteer = false;      // Use absolute steering
            float steeringIncrement = 0.01f; // Steering speed
            float steeringClamp = 0.15f;     // Max steering angle

            // Wheel (old)
            float wheelRadius{0.3f};
            float wheelWidth{0.2f};
            float wheelFriction = 0.45f;
            float suspensionRestLength = 0.020f;

            // Suspension (old)
            float suspensionStiffness = 950.f;
            float suspensionDamping = 200.f;
            float suspensionCompression = 200.4f;
            float rollInfluence = 0.04f; // Shift CoM

            // ============================================
            // NFS3/NFS4 carp.txt fields (from CarpFile)
            // ============================================
            uint8_t serialNumber{0};
            uint8_t carClassification{0};
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
        };

        enum class ModelIndex : uint8_t {
            LEFT_FRONT_WHEEL = 0,
            RIGHT_FRONT_WHEEL,
            LEFT_REAR_WHEEL,
            RIGHT_REAR_WHEEL,
            CAR_BODY
        };

        explicit Car(MetaData const &carData, NFSVersion const nfsVersion, std::string const &carID)
            : id(carID), tag(nfsVersion), metadata(carData) {};
        explicit Car(MetaData const &carData, NFSVersion const nfsVersion, std::string const &carID, PhysicsData const &physicsData)
            : id(carID), tag(nfsVersion), metadata(carData), physicsData(physicsData) {};
        Car(MetaData const &carData, NFSVersion const nfsVersion, std::string const &carID, bool const _multi_textured)
            : Car(carData, nfsVersion, carID) {
            isMultitextured = _multi_textured;
        };

        std::string id;
        NFSVersion tag;
        MetaData metadata;
        PhysicsData physicsData;
        bool isMultitextured{false};
    };
} // namespace LibOpenNFS
