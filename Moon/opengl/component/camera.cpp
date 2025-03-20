#include"../pch.h"
#include "camera.h"
#include "transform.h"
#include "../util/math.h"
using namespace utils::math;
namespace component {

    Camera::Camera(Transform* T, View view) :
        Component(),
        fov(45.0f),
        near_clip(0.1f),
        far_clip(100.0f),
        move_speed(5.0f),
        zoom_speed(0.04f),
        rotate_speed(0.3f),
        orbit_speed(0.05f),
        initial_position(T->position),
        initial_rotation(T->rotation),
        T(T), view(view) {}

    glm::mat4 Camera::GetViewMatrix() const {

        if constexpr (true) {
            return glm::inverse(T->transform);
        }
        else {
            return glm::lookAt(T->position, T->position + T->forward, T->up);
        }
    }

    glm::mat4 Camera::GetProjectionMatrix() const {
        return (view == View::Orthgraphic)
            ? glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_clip, far_clip)
            : glm::perspective(glm::radians(fov),aspect, near_clip, far_clip);
    }

    void Camera::Update() {

    }

}
