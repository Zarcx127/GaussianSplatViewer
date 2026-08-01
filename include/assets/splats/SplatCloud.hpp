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

#ifndef ASSETS_SPLATS_SPLAT_CLOUD_H
#define ASSETS_SPLATS_SPLAT_CLOUD_H

#include <vector>
#include <cstddef>
#include <cstdint>

#include <glm/glm.hpp>

struct SplatVertex
{
    glm::vec3 position {};
    glm::vec3 color { 1.0f, 1.0f, 1.0f };
    float opacity { 1.0f };

    glm::vec3 logScale {};
    glm::vec4 rotation { 0.0f, 0.0f, 0.0f, 1.0f };
};

struct SplatCloudInfo
{
    bool hasRgbColor { false };
    bool hasDcColor { false };
    bool hasOpacity { false };
    bool hasScale { false };
    bool hasRotation { false };

    uint32_t sphericalHarmonicDegree { 0 };
};

class SplatCloud
{
public:
    std::vector<SplatVertex> splats;
    std::vector<glm::vec3> sphericalHarmonics;

    SplatCloudInfo info {};

    bool empty() const;

    uint32_t splat_count() const;

    uint32_t spherical_harmonic_coefficient_count() const;

    void clear();
};

#endif
