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
