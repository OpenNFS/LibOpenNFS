#pragma once

#include <glm/glm.hpp>
#include <string>

#include "CarGeometry.h"
#include "Common/NFSVersion.h"

namespace LibOpenNFS {
    class Car {
    public:
        class Dummy {
        public:
            std::string name;
            glm::vec3 position;
            Dummy(const char* dummyName, const glm::vec3 position) {
                this->name     = std::string(dummyName);
                this->position = position;
            }
        };

        class Colour {
        public:
            std::string colourName;
            glm::vec3 colour;
            Colour(const std::string& colourName, const glm::vec3 colour) {
                this->colourName = colourName;
                this->colour     = colour;
            }
        };

        class MetaData {
        public:
            explicit MetaData() = default;
            std::string name    = "Unset";
            std::vector<Dummy> dummies;
            std::vector<Colour> colours;
            std::vector<CarGeometry> meshes;
        };

        class PhysicsData {
        public:
            explicit PhysicsData() = default;
            float mass = 1750.f;

            // Engine
            float maxEngineForce = 3000.f;   // Max engine force to apply
            float maxBreakingForce = 1000.f; // Max breaking force
            float maxSpeed = 100.f;         // Max speed before stop applying engine force

            // Steering
            bool absoluteSteer = false;      // Use absolute steering
            float steeringIncrement = 0.01f; // Steering speed
            float steeringClamp = 0.15f;     // Max steering angle

            // Wheel
            float wheelRadius;
            float wheelWidth;
            float wheelFriction = 0.45f;
            float suspensionRestLength = 0.020f;

            // Suspension
            float suspensionStiffness = 750.f;
            float suspensionDamping = 200.f;
            float suspensionCompression = 200.4f;
            float rollInfluence = 0.04f; // Shift CoM
        };

        enum class ModelIndex : uint8_t { LEFT_FRONT_WHEEL = 0, RIGHT_FRONT_WHEEL, LEFT_REAR_WHEEL, RIGHT_REAR_WHEEL, CAR_BODY };

        explicit Car(const MetaData& carData, NFSVersion nfsVersion, const std::string& carID) : metadata(carData), tag(nfsVersion), id(carID){};
        Car(const MetaData& carData, NFSVersion nfsVersion, const std::string& carID, bool _multi_textured) : Car(carData, nfsVersion, carID) {
            isMultitextured = _multi_textured;
        };

        std::string id;
        NFSVersion tag;
        MetaData metadata;
        PhysicsData physicsData;
        bool isMultitextured{false};
    };
} // namespace LibOpenNFS
