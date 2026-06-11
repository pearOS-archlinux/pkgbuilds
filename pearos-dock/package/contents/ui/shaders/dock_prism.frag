#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float cornerRadius;
    float edgeSize;
    float refractionStrength;
    float rgbFringing;
    float itemWidth;
    float itemHeight;
};

layout(binding = 1) uniform sampler2D source;

float roundedRectDist(vec2 p, vec2 halfSize, float r) {
    vec2 q = abs(p) - halfSize + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

void main() {
    vec2 uv       = qt_TexCoord0;
    vec2 size     = vec2(itemWidth, itemHeight);
    vec2 halfSize = size * 0.5;
    vec2 position = uv * size - halfSize;

    float r    = cornerRadius;
    float dist = roundedRectDist(position, halfSize, r);

    // Edge factor: 1.0 at the rim, fades to 0 inward over edgeSize pixels
    float edgeFactor = pow(clamp(1.0 + dist / edgeSize, 0.0, 1.0), 2.0);

    // Outward surface normal via SDF gradient
    float h = 1.0;
    vec2 grad = vec2(
        roundedRectDist(position + vec2(h, 0.0), halfSize, r)
        - roundedRectDist(position - vec2(h, 0.0), halfSize, r),
        roundedRectDist(position + vec2(0.0, h), halfSize, r)
        - roundedRectDist(position - vec2(0.0, h), halfSize, r)
    );
    vec2 outNormal = (length(grad) > 1e-6) ? normalize(grad) : vec2(0.0, 1.0);

    // Sample base texture unchanged — no UV distortion
    vec4 base = texture(source, uv);

    // Additive RGB prism glow at the rim (does not distort the background image)
    float prism = edgeFactor * rgbFringing * refractionStrength * 0.004;
    float rGlow = max(0.0,  outNormal.x + outNormal.y) * prism;
    float gGlow = edgeFactor * prism * 0.25;
    float bGlow = max(0.0, -outNormal.x + outNormal.y) * prism;

    vec3 col = clamp(base.rgb + vec3(rGlow, gGlow, bGlow), 0.0, 1.0);

    // Rounded-corner alpha clip (antialiased)
    float edgeAlpha = smoothstep(1.5, 0.0, dist);

    fragColor = vec4(col, base.a * edgeAlpha) * qt_Opacity;
}
