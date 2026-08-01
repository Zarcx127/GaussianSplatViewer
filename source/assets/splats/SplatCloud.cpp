#include "assets/splats/SplatCloud.hpp"

bool SplatCloud::empty() const
{
    return splats.empty();
}

uint32_t SplatCloud::splat_count() const
{
    return static_cast<uint32_t>(splats.size());
}

uint32_t SplatCloud::spherical_harmonic_coefficient_count() const
{
    if(!info.hasDcColor)
        return 0;

    uint32_t side = (info.sphericalHarmonicDegree + 1);

    return (side * side);
}

glm::vec3 SplatCloud::center() const
{
    return ((boundsMin + boundsMax) * 0.5f);
}

glm::vec3 SplatCloud::size() const
{
    return (boundsMax - boundsMin);
}

void SplatCloud::clear()
{
    splats.clear();
    sphericalHarmonics.clear();
    
    boundsMin = glm::vec3();
    boundsMax = glm::vec3();
    info = {};
}
