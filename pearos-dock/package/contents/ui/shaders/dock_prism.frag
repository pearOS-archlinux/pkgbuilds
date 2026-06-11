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

// Signed distance to rounded rectangle — positive = outside, negative = inside
float roundedRectDist(vec2 p, vec2 halfSize, float r) {
    vec2 q = abs(p) - halfSize + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

void main() {
    vec2 uv       = qt_TexCoord0;
    vec2 size     = vec2(itemWidth, itemHeight);
    vec2 halfSize = size * 0.5;

    // Fragment position in pixel space (origin at centre of item)
    vec2 position = uv * size - halfSize;

    float r    = cornerRadius;
    float dist = roundedRectDist(position, halfSize, r);

    // ── Refraction / RGB prism (adapted from liquid-gel upsample_core.glsl) ──
    // concaveFactor: 1.0 at the rim, falls off exponentially inward
    float concaveFactor = pow(clamp(1.0 + dist / edgeSize, 0.0, 1.0), 2.0);

    // Gradient of the SDF gives the outward surface normal at the rim
    float h = 1.0;
    vec2 grad = vec2(
        roundedRectDist(position + vec2(h, 0.0), halfSize, r)
        - roundedRectDist(position - vec2(h, 0.0), halfSize, r),
        roundedRectDist(position + vec2(0.0, h), halfSize, r)
        - roundedRectDist(position - vec2(0.0, h), halfSize, r)
    );
    vec2 normal = (length(grad) > 1e-6) ? -normalize(grad) : vec2(0.0, 1.0);

    // Different displacement per channel — red bends most, blue least (prism)
    float finalStrength = 0.2 * concaveFactor * refractionStrength;
    float fringe        = rgbFringing * 0.3;

    vec2 uvR = clamp(uv - normal * (finalStrength * (1.0 + fringe)) / size, vec2(0.0), vec2(1.0));
    vec2 uvG = clamp(uv - normal *  finalStrength                   / size, vec2(0.0), vec2(1.0));
    vec2 uvB = clamp(uv - normal * (finalStrength * (1.0 - fringe)) / size, vec2(0.0), vec2(1.0));

    float rC = texture(source, uvR).r;
    float gC = texture(source, uvG).g;
    float bC = texture(source, uvB).b;
    float aC = texture(source, uv ).a;

    // ── Rounded-corner alpha clip (antialiased) ────────────────────────────
    float edgeAlpha = smoothstep(1.5, 0.0, dist);

    fragColor = vec4(rC, gC, bC, aC * edgeAlpha) * qt_Opacity;
}
