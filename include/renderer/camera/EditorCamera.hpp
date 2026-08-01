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
