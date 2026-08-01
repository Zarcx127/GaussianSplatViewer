#include "assets/splats/SplatCloud.hpp"

bool SplatCloud::empty() const
{
    return splats.empty();
}

uint32_t SplatCloud::splat_count() const
{
    return static_cast<uint32_t>(splats.size());
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
    
    boundsMin = glm::vec3();
    boundsMax = glm::vec3();
    info = {};
}
