#pragma once
#include "utils/glm_utils.h"

namespace implemented {
    class Camera {
    public:
        Camera()
        {
            position = glm::vec3(0, 2, 5);
            forward = glm::vec3(0, 0, -1);
            up = glm::vec3(0, 1, 0);
            right = glm::vec3(1, 0, 0);
            distanceToTarget = 2;
            firstPerson = true;
            currentRotation = 0.0f;
        }

        ~Camera()
        { }

        void SetFirstPerson(bool isFirstPerson) {
            firstPerson = isFirstPerson;
        }

        void Set(const glm::vec3& position, const glm::vec3& center, const glm::vec3& up)
        {
            this->position = position;
            forward = glm::normalize(center - position);
            right = glm::normalize(glm::cross(forward, up));
            this->up = glm::normalize(glm::cross(right, forward));
        }

        void UpdatePosition(const glm::vec3& dronePosition, float droneRotation) {
            if (firstPerson) {
                // first person - camera urmareste pozitia dronei
                position = dronePosition + glm::vec3(0, 0.1f, 0);

                float rotationDiff = droneRotation - currentRotation;
                currentRotation += rotationDiff * 0.1f;

                forward = glm::vec3(
                    sin(currentRotation),
                    0,
                    -cos(currentRotation)
                );
            }
            else {
                // third person - camera drona de la distanta
                float smoothedRotation = currentRotation + (droneRotation - currentRotation) * 0.1f;
                currentRotation = smoothedRotation;

                forward = glm::vec3(
                    sin(smoothedRotation),
                    -0.3f,
                    -cos(smoothedRotation)
                );

                position = dronePosition
                    - forward * distanceToTarget * 1.0f
                    + glm::vec3(0, 1.0f, 0);
            }

            right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
            up = glm::normalize(glm::cross(right, forward));
        }

        glm::mat4 GetViewMatrix()
        {
            return glm::lookAt(position, position + forward, up);
        }

        glm::vec3 GetTargetPosition()
        {
            return position + forward * distanceToTarget;
        }

    public:
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 right;
        glm::vec3 up;
        float distanceToTarget;
        bool firstPerson;

    private:
        float currentRotation;
    };
}