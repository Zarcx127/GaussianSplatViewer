#include "renderer/camera/EditorCamera.hpp"

#include <cmath>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

void EditorCamera::update(float deltaTime, const InputState& input)
{
    if(!input.rightMouseDown) return;

    m_yaw -= static_cast<float>(input.mouseDeltaX) * m_mouseSensitivity;
    m_pitch -= static_cast<float>(input.mouseDeltaY) * m_mouseSensitivity;

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
        m_position + forward,
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
}

glm::mat4 EditorCamera::projection_matrix(float aspect) const
{
    glm::mat4 projection = glm::perspective(
        glm::radians(m_fov), aspect, m_nearPlane, m_farPlane
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
    direction.x = std::cos(pitchRadians) * std::cos(yawRadians);
    direction.y = std::cos(pitchRadians) * std::sin(yawRadians);
    direction.z = std::sin(pitchRadians);

    return glm::normalize(direction);
}
