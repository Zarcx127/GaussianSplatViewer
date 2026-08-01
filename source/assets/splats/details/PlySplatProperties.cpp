#include "assets/splats/details/PlySplatProperties.hpp"

void inspect_ply_splat_properties(
    const std::vector<PlyProperty>& properties,
    SplatCloudInfo& info
) {
    std::array<bool, 3> hasRgb;
    std::array<bool, 3> hasScale;
    std::array<bool, 4> hasRotation;

    bool hasOpacity = false;
    
    hasRgb.fill(false);
    hasScale.fill(false);
    hasRotation.fill(false);

    for(const PlyProperty& property : properties)
    {
        const std::string& name = property.name;

        hasRgb[0] |= ((name == "red") || (name == "r"));
        hasRgb[1] |= ((name == "green") || (name == "g"));
        hasRgb[2] |= ((name == "blue") || (name == "b"));
        
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

    info.hasScale = true;
    for(const bool& hasScaleComponent : hasScale)
        info.hasScale = (info.hasScale && hasScaleComponent);
    
    info.hasRotation = true;
    for(const bool& hasRotationComponent : hasRotation)
        info.hasRotation = (info.hasRotation && hasRotationComponent);

    info.hasOpacity = hasOpacity;
}
