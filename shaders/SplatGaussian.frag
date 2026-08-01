#version 450

layout(location = 0) in vec2 gaussianPosition;
layout(location = 1) flat in vec4 gaussianColor;

layout(location = 0) out vec4 outColor;

void main()
{
    float radiusSquared = dot(gaussianPosition, gaussianPosition);
    if(radiusSquared > 9.0)
        discard;

    float gaussian = exp(-0.5 * radiusSquared);
    float alpha = gaussianColor.a * gaussian;

    if(alpha < (1.0 / 255.0))
        discard;

    outColor = vec4(gaussianColor.rgb * alpha, alpha);
}
