/**
 * Copyright (C) 2026  Zarcx127@github.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

#include "renderer/camera/EditorCamera.hpp"

#include <cmath>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

void EditorCamera::update(float deltaTime, const InputState& input)
{
    if(!input.rightMouseDown) return;

    m_yaw -= (static_cast<float>(input.mouseDeltaX) * m_mouseSensitivity);
    m_pitch -= (static_cast<float>(input.mouseDeltaY) * m_mouseSensitivity);
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    const glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
    const glm::vec3 forward = forward_direction();
    const glm::vec3 right = glm::normalize(glm::cross(forward, up));

    glm::vec3 movement = glm::vec3(0.0f);

    if(input.keyW) movement += forward;
    if(input.keyS) movement -= forward;
    if(input.keyD) movement += right;
    if(input.keyA) movement -= right;
    if(input.keyE) movement += up;
    if(input.keyQ) movement -= up;

    if(glm::dot(movement, movement) > 0.0001f)
    {
        movement = glm::normalize(movement);
        m_position += movement * m_moveSpeed * deltaTime;
    }
}

glm::mat4 EditorCamera::view_matrix() const
{
    glm::vec3 forward = forward_direction();

    return glm::lookAt(
        m_position,
        (m_position + forward),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
}

glm::mat4 EditorCamera::projection_matrix(float aspect) const
{
    glm::mat4 projection = glm::perspective(
        glm::radians(m_fov), aspect, 
        m_nearPlane, m_farPlane
    );

    projection[1][1] *= -1.0f;

    return projection;
}

glm::vec3 EditorCamera::position() const
{
    return m_position;
}

glm::vec3 EditorCamera::forward_direction() const
{
    float yawRadians = glm::radians(m_yaw);
    float pitchRadians = glm::radians(m_pitch);

    glm::vec3 direction {};
    direction.x = (std::cos(pitchRadians) * std::cos(yawRadians));
    direction.y = (std::cos(pitchRadians) * std::sin(yawRadians));
    direction.z = std::sin(pitchRadians);

    return glm::normalize(direction);
}
