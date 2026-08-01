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

#ifndef ASSETS_SPLATS_DETAILS_PLY_SPHERICAL_HARMONICS_H
#define ASSETS_SPLATS_DETAILS_PLY_SPHERICAL_HARMONICS_H

#include <vector>
#include <string>
#include <cstdint>

#include <glm/glm.hpp>

#include "assets/ply/PlyReader.hpp"

struct PlySphericalHarmonicLayout
{
    bool hasDc { false };

    uint32_t degree { 0 };
    uint32_t coefficientCount { 0 };
};

bool inspect_ply_spherical_harmonics(
    const std::vector<PlyProperty>& properties,
    PlySphericalHarmonicLayout& layout,
    const char*& error
);

bool get_ply_spherical_harmonic_location(
    const std::string& propertyName,
    const uint32_t& coefficientCount,
    uint32_t& coefficient,
    uint32_t& component
);

glm::vec3 decode_ply_spherical_harmonic_dc_color(
    const glm::vec3& coefficient
);

#endif
