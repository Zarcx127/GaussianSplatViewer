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

#version 450
#extension GL_GOOGLE_include_directive : require

layout(push_constant) uniform LoadingScreenPushConstant
{
    layout(offset = 112) 
    float width;
    float height;
    float time;
}
loadingScreenPushConstant;

layout(location = 0) out vec4 outputColor;

const float TWO_PI = 6.28318530718;

void main()
{
    vec2 extent = vec2(
        loadingScreenPushConstant.width,
        loadingScreenPushConstant.height
    );

    vec2 position = (gl_FragCoord.xy - (extent * 0.5));

    float minimumExtent = min(
        loadingScreenPushConstant.width,
        loadingScreenPushConstant.height
    );

    float radius = (minimumExtent * 0.055);
    float thickness = max(minimumExtent * 0.008, 2.0);

    float ringDistance = abs(length(position) - radius);
    float ringEdge = max(fwidth(ringDistance), 0.75);

    float ringMask = 1.0 - smoothstep(
        (thickness - ringEdge),
        (thickness + ringEdge),
        ringDistance
    );

    float angle = atan(position.y, position.x);
    angle = mod(
        (angle - (loadingScreenPushConstant.time * 4.0) + TWO_PI),
        TWO_PI
    );

    float arcEnd = (TWO_PI * 0.78);
    float arcEdge = 0.1;

    float arcStartMask = smoothstep(0.0, arcEdge, angle);
    float arcEndMask = (1.0 - smoothstep((arcEnd - arcEdge), arcEnd, angle));
    float arcMask = (arcStartMask * arcEndMask);

    vec3 backgroundColor = (vec3(25.0, 27.0, 29.0) / 255.0);
    vec3 spinnerColor = (vec3(220.0, 220.0, 225.0) / 255.0);

    outputColor = vec4(
        mix(
            backgroundColor,
            spinnerColor,
            (ringMask * arcMask)
        ),
        1.0
    );
}