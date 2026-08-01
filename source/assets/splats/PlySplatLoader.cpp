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

#include "assets/splats/PlySplatLoader.hpp"

#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>

#include "assets/ply/details/PlyScalar.hpp"

#include "assets/ply/PlyReader.hpp"

#include "assets/splats/details/PlySplatProperties.hpp"
#include "assets/splats/details/PlySphericalHarmonics.hpp"

namespace
{
    bool apply_splat_property(
        SplatVertex& splat,
        glm::vec3& rgb,
        std::vector<glm::vec3>& sphericalHarmonics,
        const PlyProperty& property,
        double value,
        const char*& error
    );

    void resolve_splat_base_color(
        SplatVertex& splat,
        const glm::vec3& rgb,
        const std::vector<glm::vec3>& sphericalHarmonics,
        const SplatCloudInfo& info
    );

    bool normalize_color(float& component, double value, const PlyScalarInfo& info);
    bool assign_splat_float(float& destination, double value, const char*& error);
    bool assign_spherical_harmonic(
        std::vector<glm::vec3>& sphericalHarmonics,
        const std::string& name,
        double value,
        const char*& error
    );

    float clamp01(float value);
    float sigmoid(double value);
}

PlySplatLoadResult load_ply_splat_cloud(
    const std::filesystem::path& path,
    const std::function<bool()>& progressCallback
) {
    PlySplatLoadResult result = {};

    PlyReader reader;
    if(!reader.open(path, result.error))
        return result;

    if(reader.vertex_count() > std::numeric_limits<uint32_t>::max())
    {
        result.error = "PLY vertex count exceeds supported splat count";
        return result;
    }

    PlySphericalHarmonicLayout sphericalHarmonicLayout = {};

    if(!inspect_ply_spherical_harmonics(
        reader.vertex_properties(), sphericalHarmonicLayout, result.error
    )) {
        return result;
    }

    inspect_ply_splat_properties(
        reader.vertex_properties(), 
        result.cloud.info
    );

    result.cloud.info.hasDcColor= sphericalHarmonicLayout.hasDc;
    result.cloud.info.sphericalHarmonicDegree = sphericalHarmonicLayout.degree;

    if(
        !result.cloud.info.hasRgbColor &&
        !result.cloud.info.hasDcColor
    ) {
        result.error = "PLY splat data is missing color";
        return result;
    }

    if(!result.cloud.info.hasOpacity)
    {
        result.error = "PLY splat data is missing opacity";
        return result;
    }

    if(!result.cloud.info.hasScale)
    {
        result.error = "PLY splat data is missing scale";
        return result;
    }

    if(!result.cloud.info.hasRotation)
    {
        result.error = "PLY splat data is missing rotation";
        return result;
    }

    uint32_t sphericalCoefficientCount = sphericalHarmonicLayout.coefficientCount;

    result.cloud.splats.reserve(reader.vertex_count());
    result.cloud.sphericalHarmonics.reserve(
        reader.vertex_count() * sphericalCoefficientCount
    );

    std::vector<glm::vec3> sphericalHarmonics(sphericalCoefficientCount);

    for(uint64_t vertexIndex = 0; vertexIndex < reader.vertex_count(); vertexIndex++)
    {
        if((vertexIndex % 4096) == 0)
        {
            if(progressCallback && !progressCallback())
            {
                result.cloud.clear();
                return result;
            }
        }

        SplatVertex splat = {};
        glm::vec3 rgb = {};

        for(const PlyProperty& property : reader.vertex_properties())
        {
            double value = 0.0;
            if(!reader.read_scalar(property.scalar, value, result.error))
            {
                result.cloud.clear();
                return result;
            }

            if(!apply_splat_property(
                splat, rgb, sphericalHarmonics, 
                property, value, result.error
            )) {
                result.cloud.clear();
                return result;
            }
        }

        splat.position = glm::vec3(
            splat.position.x,
            splat.position.z,
            -splat.position.y
        );

        resolve_splat_base_color(splat, rgb, sphericalHarmonics, result.cloud.info);

        result.cloud.sphericalHarmonics.insert(
            result.cloud.sphericalHarmonics.end(),
            sphericalHarmonics.begin(),
            sphericalHarmonics.end()
        );

        result.cloud.splats.push_back(splat);
    }

    result.success = true;
    result.error = nullptr;

    return result;
}

namespace
{
    bool apply_splat_property(
        SplatVertex& splat,
        glm::vec3& rgb,
        std::vector<glm::vec3>& sphericalHarmonics,
        const PlyProperty& property,
        double value,
        const char*& error
    ) {
        const std::string& name = property.name;

        if((name == "red") || (name == "r"))
            return normalize_color(rgb.r, value, property.scalar);
        if((name == "green") || (name == "g"))
            return normalize_color(rgb.g, value, property.scalar);
        if((name == "blue") || (name == "b"))
            return normalize_color(rgb.b, value, property.scalar);
        
        if(name == "x")
            return assign_splat_float(splat.position.x, value, error);
        if(name == "y")
            return assign_splat_float(splat.position.y, value, error);
        if(name == "z")
            return assign_splat_float(splat.position.z, value, error);
        
        if(name == "f_dc_0")
            return assign_splat_float(sphericalHarmonics[0].r, value, error);
        if(name == "f_dc_1")
            return assign_splat_float(sphericalHarmonics[0].g, value, error);
        if(name == "f_dc_2")
            return assign_splat_float(sphericalHarmonics[0].b, value, error);

        if(name.compare(0, 7, "f_rest_") == 0)
            return assign_spherical_harmonic(
                sphericalHarmonics, name, value, error
            );

        if(name == "scale_0")
            return assign_splat_float(splat.logScale.x, value, error);
        if(name == "scale_1")
            return assign_splat_float(splat.logScale.y, value, error);
        if(name == "scale_2")
            return assign_splat_float(splat.logScale.z, value, error);

        if(name == "rot_0")
            return assign_splat_float(splat.rotation.w, value, error);
        if(name == "rot_1")
            return assign_splat_float(splat.rotation.x, value, error);
        if(name == "rot_2")
            return assign_splat_float(splat.rotation.y, value, error);
        if(name == "rot_3")
            return assign_splat_float(splat.rotation.z, value, error);

        if((name == "alpha") || (name == "a"))
            normalize_color(splat.opacity, value, property.scalar);

        if(name == "opacity")
        {
            if(property.scalar.category == PlyScalarCategory::FloatingPoint)
            {
                splat.opacity = sigmoid(value);
                return true;
            }

            return normalize_color(splat.opacity, value, property.scalar);
        }
        
        return true;
    }

    void resolve_splat_base_color(
        SplatVertex& splat,
        const glm::vec3& rgb,
        const std::vector<glm::vec3>& sphericalHarmonics,
        const SplatCloudInfo& info
    ) {
        if(info.hasRgbColor)
        {
            splat.color = rgb;
            return;
        }

        if(info.hasDcColor)
        {
            splat.color = decode_ply_spherical_harmonic_dc_color(
                sphericalHarmonics[0]
            );

            return;
        }
    }

    bool normalize_color(float& component, double value, const PlyScalarInfo& info)
    {
        if(info.category == PlyScalarCategory::FloatingPoint)
            component = clamp01(value);
        else 
            component = clamp01(value / info.maximum);

        return true;
    }

    bool assign_splat_float(float& destination, double value, const char*& error)
    {
        if(
            (value < static_cast<double>(std::numeric_limits<float>::lowest())) ||
            (value > static_cast<double>(std::numeric_limits<float>::max()))
        ) {
            error = "PLY splat property exceeds 32-bit float range";
            return false;
        }

        destination = static_cast<float>(value);

        return true;
    }

    bool assign_spherical_harmonic(
        std::vector<glm::vec3>& sphericalHarmonics,
        const std::string& name,
        double value,
        const char*& error
    ) {
        uint32_t coefficient = 0;
        uint32_t component = 0;

        if(!get_ply_spherical_harmonic_location(
            name, static_cast<uint32_t>(sphericalHarmonics.size()),
            coefficient, component
        )) {
            error = "Invalid spherical harmonic property";
            return false;
        }

        return assign_splat_float(
            sphericalHarmonics[coefficient][component],
            value, error
        );
    }

    float clamp01(float value)
    {
        return std::clamp(value, 0.0f, 1.0f);
    }

    float sigmoid(double value)
    {
        if(value >= 0.0)
        {
            double exponent = std::exp(-value);
            return static_cast<float>(1.0 / (1.0 + exponent));
        }

        double exponent = std::exp(value);
        return static_cast<float>(exponent / (1.0 + exponent));
    }
}
