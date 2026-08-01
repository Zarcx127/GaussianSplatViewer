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
