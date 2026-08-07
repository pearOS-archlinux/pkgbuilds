#version 140

// Fragment shader for the pearOS window tinter (core profile).
//
// KWin generates the vertex stage from the shader traits we pass to
// generateShaderFromFile(), so texcoord0 and the modulation/saturation
// uniforms below follow the names of KWin's own built-in shaders --
// see ShaderManager::generateFragmentSource() in libkwin.
//
// The uniform initializers matter: if a uniform ends up unused by the
// draw path that binds this shader, we still want a neutral value rather
// than black.

uniform sampler2D sampler;
uniform vec4 modulation = vec4(1.0);
uniform float saturation = 1.0;
uniform vec3 primaryBrightness = vec3(0.2126, 0.7152, 0.0722);

// Wallpaper color sampled behind this window, and how far to pull the
// background towards it (0..1).
uniform vec3 tintColor = vec3(0.0);
uniform float tintAmount = 0.0;

// One texel of the window's offscreen texture, for the local contrast probe.
uniform vec2 texelSize = vec2(0.0);

// Guards: each scales the tint down where the pixel does not look like
// flat window background. See README.md for the reasoning.
uniform float chromaGuard = 1.0;
uniform vec2 chromaRange = vec2(0.10, 0.35);
uniform float edgeGuard = 1.0;
uniform vec2 edgeRange = vec2(0.02, 0.12);

// Optional color key against a known background color.
uniform float keyStrength = 0.0;
uniform vec3 keyColor = vec3(0.0);
uniform vec2 keyRange = vec2(0.12, 0.15); // (tolerance, softness)

in vec2 texcoord0;
out vec4 fragColor;

float luma(vec3 c)
{
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

// Luma of a neighbouring texel, kept premultiplied: alpha barely varies
// across a few texels, and skipping the divide keeps the probe cheap.
float neighbourLuma(vec2 offset)
{
    return luma(texture(sampler, clamp(texcoord0 + offset, vec2(0.0), vec2(1.0))).rgb);
}

void main(void)
{
    vec4 tex = texture(sampler, texcoord0);

    // The offscreen texture covers the window's *expanded* geometry, so it
    // includes the shadow and the transparent area outside rounded corners.
    // Dividing by a near-zero alpha there produces huge bogus colors, the
    // guards read them as content, and the result is a bright fringe along
    // the window border. Leave anything that thin alone entirely.
    if (tex.a < 0.004) {
        fragColor = tex * modulation;
        return;
    }

    // Analysis happens on straight (un-premultiplied) color, otherwise a
    // translucent window reads as "dark" and the guards misfire.
    float alpha = tex.a;
    vec3 straight = clamp(tex.rgb / alpha, 0.0, 1.0);

    float weight = tintAmount;
    // Fade the tint out across the antialiased edge and into the shadow, so
    // the border keeps the exact pixels the decoration drew.
    weight *= smoothstep(0.0, 0.25, alpha);

    // Saturated pixels are icons, thumbnails, photos -- never background.
    float chroma = max(max(straight.r, straight.g), straight.b)
                 - min(min(straight.r, straight.g), straight.b);
    weight *= mix(1.0, 1.0 - smoothstep(chromaRange.x, chromaRange.y, chroma), chromaGuard);

    // High local contrast is text and edges. Flat background has none.
    float center = luma(tex.rgb);
    float gradient = abs(neighbourLuma(vec2(texelSize.x, 0.0)) - center)
                   + abs(neighbourLuma(vec2(-texelSize.x, 0.0)) - center)
                   + abs(neighbourLuma(vec2(0.0, texelSize.y)) - center)
                   + abs(neighbourLuma(vec2(0.0, -texelSize.y)) - center);
    weight *= mix(1.0, 1.0 - smoothstep(edgeRange.x, edgeRange.y, gradient), edgeGuard);

    // Optional: only tint what is close to a known background color.
    float keyDistance = distance(straight, keyColor);
    weight *= mix(1.0, 1.0 - smoothstep(keyRange.x, keyRange.x + keyRange.y, keyDistance), keyStrength);

    vec3 tinted = mix(straight, tintColor, clamp(weight, 0.0, 1.0));
    vec4 result = vec4(tinted * alpha, tex.a);

    // Same tail as KWin's generated shader: saturation, then modulation.
    float Y = dot(result.rgb, primaryBrightness);
    result = vec4(mix(vec3(Y), result.rgb, saturation), result.a);
    result *= modulation;

    fragColor = result;
}
