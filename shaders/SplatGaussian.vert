#version 450

layout(location = 0) in vec3 splatPosition;
layout(location = 1) in vec4 splatColor;
layout(location = 2) in vec3 splatLogScale;
layout(location = 3) in vec4 splatRotation;

layout(set = 1, binding = 0, std430) readonly buffer SphericalHarmonicBuffer
{
    vec4 coefficients[];
} sphericalHarmonicBuffer;

layout(location = 0) out vec2 gaussianPosition;
layout(location = 1) flat out vec4 gaussianColor;

layout(push_constant) uniform PushConstants
{
    mat4 view;
    mat4 projection;
    vec4 cameraPosition;
    uvec4 renderInfo;
} pushConstants;

const float SIGMA_EXTENT = 3.0;

const uint MAX_SUPPORTED_SH_DEGREE = 3;

const float SH0 = 0.28209479177387814;
const float SH1 = 0.4886025119029199;

const float SH2[5] = float[5](
    1.0925484305920792,
    -1.0925484305920792,
    0.31539156525252005,
    -1.0925484305920792,
    0.5462742152960396
);

const float SH3[7] = float[7](
    -0.5900435899266435,
    2.890611442640554,
    -0.4570457994644658,
    0.3731763325901154,
    -0.4570457994644658,
    1.445305721320277,
    -0.5900435899266435
);

const mat3 PLY_TO_ENGINE = mat3(
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 0.0, -1.0),
    vec3(0.0, 1.0, 0.0)
);

vec2 quad_corner(uint vertexIndex);

mat3 quaternion_matrix(vec4 quaternion);

vec3 spherical_harmonic(uint coefficient);

vec3 evaluate_spherical_harmonics(
    uint degree,
    vec3 direction
);

void main()
{
    vec4 viewCenter = pushConstants.view * vec4(splatPosition, 1.0);
    float viewDepth = -viewCenter.z;

    if(viewDepth <= 0.1)
    {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        gaussianPosition = vec2(0.0);
        gaussianColor = vec4(0.0);
        return;
    }

    vec3 scale = exp(splatLogScale);
    mat3 rotation = PLY_TO_ENGINE * quaternion_matrix(splatRotation);

    mat3 scaleSquared = mat3(
        vec3(scale.x * scale.x, 0.0, 0.0),
        vec3(0.0, scale.y * scale.y, 0.0),
        vec3(0.0, 0.0, scale.z * scale.z)
    );

    mat3 worldCovariance = rotation * scaleSquared * transpose(rotation);
    mat3 viewRotation = mat3(pushConstants.view);
    mat3 viewCovariance =
        viewRotation * worldCovariance * transpose(viewRotation);

    float focalX = 
        0.5 * float(pushConstants.renderInfo.x) * pushConstants.projection[0][0];

    float focalY =
        0.5 * float(pushConstants.renderInfo.y) * pushConstants.projection[1][1];

    float inverseDepth = 1.0 / viewDepth;
    float inverseDepthSquared = inverseDepth * inverseDepth;

    vec3 horizontalDerivative = vec3(
        focalX * inverseDepth,
        0.0,
        focalX * viewCenter.x * inverseDepthSquared
    );

    vec3 verticalDerivative = vec3(
        0.0,
        focalY * inverseDepth,
        focalY * viewCenter.y * inverseDepthSquared
    );

    float covarianceXX =
        dot(horizontalDerivative, viewCovariance * horizontalDerivative) + 0.3;

    float covarianceXY =
        dot(horizontalDerivative, viewCovariance * verticalDerivative);

    float covarianceYY =
        dot(verticalDerivative, viewCovariance * verticalDerivative) + 0.3;

    float midpoint = 0.5 * (covarianceXX + covarianceYY);
    float radius = length(
        vec2(
            0.5 * (covarianceXX - covarianceYY),
            covarianceXY
        )
    );

    float majorValue = max(midpoint + radius, 0.0001);
    float minorValue = max(midpoint - radius, 0.0001);

    vec2 majorDirection;

    if(abs(covarianceXY) > 0.00001)
        majorDirection = normalize(vec2(covarianceXY, majorValue - covarianceXX));
    else if(covarianceXX >= covarianceYY)
        majorDirection = vec2(1.0, 0.0);
    else
        majorDirection = vec2(0.0, 1.0);

    vec2 minorDirection = vec2(-majorDirection.y, majorDirection.x);

    vec2 majorAxis =
        majorDirection * sqrt(majorValue) * SIGMA_EXTENT;

    vec2 minorAxis =
        minorDirection * sqrt(minorValue) * SIGMA_EXTENT;

    vec2 corner = quad_corner(gl_VertexIndex);
    vec2 pixelOffset = (corner.x * majorAxis) + (corner.y * minorAxis);

    vec2 viewport = vec2(pushConstants.renderInfo.xy);
    vec2 ndcOffset = pixelOffset * (2.0 / viewport);

    vec4 clipCenter = pushConstants.projection * viewCenter;

    gl_Position = clipCenter;
    gl_Position.xy += ndcOffset * clipCenter.w;

    gaussianPosition = corner * SIGMA_EXTENT;

    vec3 color = splatColor.rgb;
    if(pushConstants.renderInfo.w > 0)
    {
        vec3 direction = splatPosition - pushConstants.cameraPosition.xyz;
        float directionLengthSquared = dot(direction, direction);

        if(directionLengthSquared > 0.0)
        {
            direction *= inversesqrt(directionLengthSquared);
            direction = transpose(PLY_TO_ENGINE) * direction;

            uint degree = min(
                pushConstants.renderInfo.z,
                MAX_SUPPORTED_SH_DEGREE
            );

            color = evaluate_spherical_harmonics(degree, direction);
        }
    }

    gaussianColor = vec4(color, splatColor.a);
}

vec2 quad_corner(uint vertexIndex)
{
    const vec2 corners[6] = vec2[6] (
        vec2(-1.0, -1.0),
        vec2(1.0, -1.0),
        vec2(1.0, 1.0),
        vec2(-1.0, -1.0),
        vec2(1.0, 1.0),
        vec2(-1.0, 1.0)
    );

    return corners[vertexIndex];
}

mat3 quaternion_matrix(vec4 quaternion)
{
    quaternion = normalize(quaternion);

    float x = quaternion.x;
    float y = quaternion.y;
    float z = quaternion.z;
    float w = quaternion.w;

    return mat3(
        vec3(
            1.0 - (2.0 * ((y * y) + (z * z))),
            2.0 * ((x * y) + (z * w)),
            2.0 * ((x * z) - (y * w))
        ),
        vec3(
            2.0 * ((x * y) - (z * w)),
            1.0 - (2.0 * ((x * x) + (z * z))),
            2.0 * ((y * z) + (x * w))
        ),
        vec3(
            2.0 * ((x * z) + (y * w)),
            2.0 * ((y * z) - (x * w)),
            1.0 - (2.0 * ((x * x) + (y * y)))
        )
    );
}

vec3 spherical_harmonic(uint coefficient)
{
    uint coefficientCount = pushConstants.renderInfo.w;
    uint index = (gl_InstanceIndex * coefficientCount) + coefficient;

    return sphericalHarmonicBuffer.coefficients[index].rgb;
}

vec3 evaluate_spherical_harmonics(uint degree, vec3 direction)
{
    float x = direction.x;
    float y = direction.y;
    float z = direction.z;

    vec3 result = SH0 * spherical_harmonic(0);

    if(degree > 0)
    {
        result += (
            (-SH1 * y * spherical_harmonic(1)) +
            ( SH1 * z * spherical_harmonic(2)) +
            (-SH1 * x * spherical_harmonic(3))
        );
    }

    if(degree > 1)
    {
        float xx = x * x;
        float yy = y * y;
        float zz = z * z;
        float xy = x * y;
        float yz = y * z;
        float xz = x * z;

        result += (
            (SH2[0] * xy * spherical_harmonic(4)) +
            (SH2[1] * yz * spherical_harmonic(5)) +
            (SH2[2] * ((2.0 * zz) - xx - yy) * spherical_harmonic(6)) +
            (SH2[3] * xz * spherical_harmonic(7)) +
            (SH2[4] * (xx - yy) * spherical_harmonic(8))
        );

        if(degree > 2)
        {
            result += (
                (SH3[0] * y * ((3.0 * xx) - yy) * spherical_harmonic(9)) +
                (SH3[1] * xy * z * spherical_harmonic(10)) +
                (SH3[2] * y * ((4.0 * zz) - xx - yy) * spherical_harmonic(11)) +
                (SH3[3] * z * ((2.0 * zz) - (3.0 * xx) - (3.0 * yy)) * spherical_harmonic(12)) +
                (SH3[4] * x * ((4.0 * zz) - xx - yy) * spherical_harmonic(13)) +
                (SH3[5] * z * (xx - yy) * spherical_harmonic(14)) +
                (SH3[6] * x * (xx - (3.0 * yy)) * spherical_harmonic(15))
            );
        }
    }

    return max(result + vec3(0.5), vec3(0.0));
}
