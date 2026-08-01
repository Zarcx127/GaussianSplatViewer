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

    const char* sourcePath { nullptr };
};

class SplatCloud
{
public:
    std::vector<SplatVertex> splats;
    std::vector<glm::vec3> sphericalHarmonics;

    glm::vec3 boundsMin {};
    glm::vec3 boundsMax {};

    SplatCloudInfo info {};

    bool empty() const;

    uint32_t splat_count() const;

    uint32_t spherical_harmonic_coefficient_count() const;

    glm::vec3 center() const;

    glm::vec3 size() const;

    void clear();
};

#endif
