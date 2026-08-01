#include "assets/splats/details/PlySphericalHarmonics.hpp"

#include <array>
#include <limits>
#include <vector>
#include <algorithm>

namespace
{
    bool parse_spherical_harmonic_index(const std::string& name, uint32_t& index);
}

bool inspect_ply_spherical_harmonics(
    const std::vector<PlyProperty>& properties,
    PlySphericalHarmonicLayout& layout,
    const char*& error
) {
    std::array<bool, 3> hasDc {};
    std::vector<uint32_t> restIndices;

    layout = {};

    for(const PlyProperty& property : properties)
    {
        const std::string& name = property.name;

        hasDc[0] |= (name == "f_dc_0");
        hasDc[1] |= (name == "f_dc_1");
        hasDc[2] |= (name == "f_dc_2");

        if(name.compare(0, 7, "f_rest_") != 0)
            continue;

        uint32_t index = 0;
        if(!parse_spherical_harmonic_index(name, index))
        {
            error = "Invalid spherical harmonic property name";
            return false;
        }

        restIndices.push_back(index);
    }

    bool hasAnyDc = (hasDc[0] || hasDc[1] || hasDc[2]);
    layout.hasDc = (hasDc[0] && hasDc[1] && hasDc[2]);

    if(hasAnyDc && !layout.hasDc)
    {
        error = "PLY contains an incomplete spherical harmonic DC coefficient";
        return false;
    }

    std::sort(restIndices.begin(), restIndices.end());
    for(uint32_t index = 0; index < restIndices.size(); index++)
    {
        if(restIndices[index] != index)
        {
            error = "PLY spherical harmonic are incomplete or duplicated";
            return false;
        }
    }

    uint32_t restValueCount = static_cast<uint32_t>(restIndices.size());
    if(restValueCount == 0)
    {
        layout.coefficientCount = (layout.hasDc ? 1 : 0);
        return true;
    }

    if(!layout.hasDc)
    {
        error = "PLY spherical harmonic data is missing its DC coefficient";
        return false;
    }

    if((restValueCount % 3) != 0)
    {
        error = "PLY spherical harmonic channel counts do not match";
        return false;
    }

    layout.coefficientCount = (
        1 + (static_cast<uint32_t>(restValueCount) / 3)
    );

    uint64_t side = 1;
    while((side * side) < layout.coefficientCount)
        side++;
    
    if((side * side) != layout.coefficientCount)
    {
        error = "PLY spherical harmonic count is invalid";
        return false;
    }

    layout.degree = (static_cast<uint32_t>(side) - 1);

    return true;
}

bool get_ply_spherical_harmonic_location(
    const std::string& propertyName,
    const uint32_t& coefficientCount,
    uint32_t& coefficient,
    uint32_t& component
) {
    uint32_t propertyIndex = 0;
    if(!parse_spherical_harmonic_index(propertyName, propertyIndex))
        return false;
    
    if(coefficientCount <= 1)
        return false;

    uint32_t restCoefficient = (coefficientCount - 1);

    component = (propertyIndex / restCoefficient);
    coefficient = (1 + (propertyIndex % restCoefficient));

    return (component < 3);
}

namespace
{
    bool parse_spherical_harmonic_index(const std::string& name, uint32_t& index)
    {
        if(
            (name.size() <= 7) ||
            (name.compare(0, 7, "f_rest_") != 0)
        ) {
            return false;
        }

        index = 0;
        for(uint32_t position = 7; position < name.size(); position++)
        {
            char digit = name[position];
            if((digit < '0') || (digit > '9'))
                return false;
            
            uint32_t value = static_cast<uint32_t>(digit - '0');
            if(index > ((std::numeric_limits<uint32_t>::max() - value) / 10))
                return false;

            index = (index * 10) + value;
        }

        return true;
    }
}
