

#pragma once

#include <glm/glm.hpp>
#include "component.h"

namespace component {

    enum class Space : char {
        Local = 1 << 0,
        World = 1 << 1
    };

    class Transform : public Component {
      private:
        void RecalculateBasis(void);
        void RecalculateEuler(void);

      public:
        glm::vec3 position;
        glm::quat rotation;   // rotations are internally represented as quaternions
        glm::mat4 transform;  // 4x4 homogeneous matrix stored in column-major order

        float euler_x, euler_y, euler_z;  // euler angles in degrees (pitch, yawn, roll)
        float scale_x, scale_y, scale_z;

        glm::vec3 up;
        glm::vec3 forward;
        glm::vec3 right;

        Transform();

        void Translate(const glm::vec3& vector, Space space = Space::World);
        void Translate(float x, float y, float z, Space space = Space::World);

        void Rotate(const glm::vec3& axis, float angle, Space space);
        void Rotate(const glm::vec3& eulers, Space space);
        void Rotate(float euler_x, float euler_y, float euler_z, Space space);

        void Scale(float scale);
        void Scale(const glm::vec3& scale);
        void Scale(float scale_x, float scale_y, float scale_z);

        void SetPosition(const glm::vec3& position);
        void SetRotation(const glm::quat& rotation);
        void SetTransform(const glm::mat4& transform);

        // converts a vector from local to world space or vice versa
        glm::vec3 Local2World(const glm::vec3& v) const;
        glm::vec3 World2Local(const glm::vec3& v) const;

        // returns the local tranform matrix that converts world space to local space
        glm::mat4 GetLocalTransform() const;
        glm::mat4 GetLocalTransform(const glm::vec3& forward, const glm::vec3& up) const;
    };

}