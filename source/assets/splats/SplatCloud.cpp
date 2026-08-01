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

void SplatCloud::clear()
{
    splats.clear();
    sphericalHarmonics.clear();

    info = {};
}
