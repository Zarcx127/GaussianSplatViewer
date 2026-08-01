#pragma once

#ifndef RENDERER_CAMERA_EDITOR_CAMERA_H
#define RENDERER_CAMERA_EDITOR_CAMERA_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "input/InputState.hpp"

class EditorCamera
{
public:
    void update(float deltaTime, const InputState& input);

    glm::mat4 view_matrix() const;
    glm::mat4 projection_matrix(float aspect) const;

    glm::vec3 position() const;

private:
    glm::vec3 m_position  { 2.0f, 2.0f, 2.0f };
    
    float m_yaw { -135.0f };
    float m_pitch { -35.0f };

    float m_fov { 45.0f };
    float m_nearPlane { 0.1f };
    float m_farPlane { 1000.0f };

    float m_moveSpeed { 2.5f };
    float m_mouseSensitivity { 0.1f };

    glm::vec3 forward_direction() const;
};

#endif
