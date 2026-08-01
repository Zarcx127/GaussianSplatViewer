#include "assets/splats/PlySplatLoader.hpp"

#include <array>
#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>

#include "assets/ply/details/PlyScalar.hpp"

#include "assets/ply/PlyReader.hpp"

namespace
{
    SplatCloudInfo make_splat_info(const std::vector<PlyProperty>& properties);

    bool apply_splat_property(
        SplatVertex& splat,
        glm::vec3& rgb,
        glm::vec3& dc,
        const PlyProperty& property,
        double value,
        const char*& error
    );

    void finalize_splat_color(
        SplatVertex& splat,
        const glm::vec3& rgb,
        const glm::vec3& dc,
        const SplatCloudInfo& info
    );

    bool normalize_color(float& component, double value, const PlyScalarInfo& info);
    bool assign_splat_float(float& destination, double value, const char*& error);

    void grow_bounds(SplatCloud& cloud, const glm::vec3& position, bool firstSplat);

    float clamp01(float value);
    float sigmoid(double value);
}

PlySplatLoadResult load_ply_splat_cloud(const char* path)
{
    PlySplatLoadResult result = {};

    PlyReader reader;
    if(!reader.open(path, result.error))
        return result;

    if(reader.vertex_count() > std::numeric_limits<uint32_t>::max())
    {
        result.error = "PLY vertex count exceeds supported splat count";
        return result;
    }

    result.cloud.info = make_splat_info(reader.vertex_properties());
    result.cloud.splats.reserve(reader.vertex_count());

    for(uint64_t vertexIndex = 0; vertexIndex < reader.vertex_count(); vertexIndex++)
    {
        SplatVertex splat = {};
        glm::vec3 rgb = {};
        glm::vec3 dc = {};

        for(const PlyProperty& property : reader.vertex_properties())
        {
            double value = 0.0;
            if(!reader.read_scalar(property.scalar, value, result.error))
            {
                result.cloud.clear();
                return result;
            }

            if(!apply_splat_property(splat, rgb, dc, property, value, result.error))
            {
                result.cloud.clear();
                return result;
            }
        }

        splat.position = glm::vec3(
            splat.position.x,
            splat.position.z,
            -splat.position.y
        );

        finalize_splat_color(splat, rgb, dc, result.cloud.info);
        grow_bounds(result.cloud, splat.position, result.cloud.splats.empty());

        result.cloud.splats.push_back(splat);
    }

    result.cloud.info.sourcePath = path;
    result.success = true;
    result.error = nullptr;

    return result;
}

namespace
{
    SplatCloudInfo make_splat_info(const std::vector<PlyProperty>& properties)
    {
        SplatCloudInfo info = {};

        std::array<bool, 3> hasRgb;
        std::array<bool, 3> hasDc;
        std::array<bool, 3> hasScale;
        std::array<bool, 4> hasRotation;
        
        bool hasOpacity = false;
        
        hasRgb.fill(false);
        hasDc.fill(false);
        hasScale.fill(false);
        hasRotation.fill(false);

        for(const PlyProperty& property : properties)
        {
            const std::string& name = property.name;

            hasRgb[0] |= ((name == "red") || (name == "r"));
            hasRgb[1] |= ((name == "green") || (name == "g"));
            hasRgb[2] |= ((name == "blue") || (name == "b"));
            
            hasDc[0] |= (name == "f_dc_0");
            hasDc[1] |= (name == "f_dc_1");
            hasDc[2] |= (name == "f_dc_2");
            
            hasScale[0] |= (name == "scale_0");
            hasScale[1] |= (name == "scale_1");
            hasScale[2] |= (name == "scale_2");
            
            hasRotation[0] |= (name == "rot_0");
            hasRotation[1] |= (name == "rot_1");
            hasRotation[2] |= (name == "rot_2");
            hasRotation[3] |= (name == "rot_3");
            
            hasOpacity |= (
                (name == "opacity") || 
                (name == "alpha") || 
                (name == "a")
            );
        }

        info.hasRgbColor = true;
        for(const bool& hasColorComponent : hasRgb)
            info.hasRgbColor = (info.hasRgbColor && hasColorComponent);

        info.hasDcColor = true;
        for(const bool& hasDcComponent : hasDc)
            info.hasDcColor = (info.hasDcColor && hasDcComponent);

        info.hasScale = true;
        for(const bool& hasScaleComponent : hasScale)
            info.hasScale = (info.hasScale && hasScaleComponent);
        
        info.hasRotation = true;
        for(const bool& hasRotationComponent : hasRotation)
            info.hasRotation = (info.hasRotation && hasRotationComponent);

        info.hasOpacity = hasOpacity;

        return info;
    }

    bool apply_splat_property(
        SplatVertex& splat,
        glm::vec3& rgb,
        glm::vec3& dc,
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
            return assign_splat_float(dc.x, value, error);
        if(name == "f_dc_1")
            return assign_splat_float(dc.y, value, error);
        if(name == "f_dc_2")
            return assign_splat_float(dc.z, value, error);

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

    void finalize_splat_color(
        SplatVertex& splat,
        const glm::vec3& rgb,
        const glm::vec3& dc,
        const SplatCloudInfo& info
    ) {
        if(info.hasRgbColor)
        {
            splat.color = rgb;
            return;
        }

        if(info.hasDcColor)
        {
            // 0th order Spherical-harmonic = 1/(2*sqrt(pi)) 
            constexpr float SH0 = 0.28209479177387814f;

            splat.color = glm::vec3(
                clamp01(0.5f + (SH0 * dc.x)),
                clamp01(0.5f + (SH0 * dc.y)),
                clamp01(0.5f + (SH0 * dc.z))
            );

            return;
        }
    }

    bool normalize_color(float& component, double value, const PlyScalarInfo& info)
    {
        if(info.category == PlyScalarCategory::FloatingPoint)
            component = clamp01(value);
        else 
            component = clamp01((value / info.maximum));

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

    void grow_bounds(SplatCloud& cloud, const glm::vec3& position, bool firstSplat)
    {
        if(firstSplat)
        {
            cloud.boundsMin = position;
            cloud.boundsMax = position;
            
            return;
        }

        cloud.boundsMin.x = std::min(cloud.boundsMin.x, position.x);
        cloud.boundsMin.y = std::min(cloud.boundsMin.y, position.y);
        cloud.boundsMin.z = std::min(cloud.boundsMin.z, position.z);
    
        cloud.boundsMax.x = std::max(cloud.boundsMax.x, position.x);
        cloud.boundsMax.y = std::max(cloud.boundsMax.y, position.y);
        cloud.boundsMax.z = std::max(cloud.boundsMax.z, position.z);
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
