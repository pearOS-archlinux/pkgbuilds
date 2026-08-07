// Fragment shader for the pearOS window tinter (legacy/GLES profile).
// Keep in sync with tint_core.frag -- KWin picks this file when the
// compositor is not on a core profile context (generateShaderFromFile()
// appends "_core" to the basename when it is).

uniform sampler2D sampler;
uniform vec4 modulation;
uniform float saturation;
uniform vec3 primaryBrightness;

uniform vec3 tintColor;
uniform float tintAmount;
uniform vec2 texelSize;

uniform float chromaGuard;
uniform vec2 chromaRange;
uniform float edgeGuard;
uniform vec2 edgeRange;

uniform float keyStrength;
uniform vec3 keyColor;
uniform vec2 keyRange;

varying vec2 texcoord0;

float luma(vec3 c)
{
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

float neighbourLuma(vec2 offset)
{
    return luma(texture2D(sampler, clamp(texcoord0 + offset, vec2(0.0), vec2(1.0))).rgb);
}

void main(void)
{
    vec4 tex = texture2D(sampler, texcoord0);

    // See tint_core.frag: near-zero alpha (shadow, outside rounded corners)
    // blows up the unpremultiply and fringes the window border.
    if (tex.a < 0.004) {
        gl_FragColor = tex * modulation;
        return;
    }

    float alpha = tex.a;
    vec3 straight = clamp(tex.rgb / alpha, 0.0, 1.0);

    float weight = tintAmount;
    weight *= smoothstep(0.0, 0.25, alpha);

    float chroma = max(max(straight.r, straight.g), straight.b)
                 - min(min(straight.r, straight.g), straight.b);
    weight *= mix(1.0, 1.0 - smoothstep(chromaRange.x, chromaRange.y, chroma), chromaGuard);

    float center = luma(tex.rgb);
    float gradient = abs(neighbourLuma(vec2(texelSize.x, 0.0)) - center)
                   + abs(neighbourLuma(vec2(-texelSize.x, 0.0)) - center)
                   + abs(neighbourLuma(vec2(0.0, texelSize.y)) - center)
                   + abs(neighbourLuma(vec2(0.0, -texelSize.y)) - center);
    weight *= mix(1.0, 1.0 - smoothstep(edgeRange.x, edgeRange.y, gradient), edgeGuard);

    float keyDistance = distance(straight, keyColor);
    weight *= mix(1.0, 1.0 - smoothstep(keyRange.x, keyRange.x + keyRange.y, keyDistance), keyStrength);

    vec3 tinted = mix(straight, tintColor, clamp(weight, 0.0, 1.0));
    vec4 result = vec4(tinted * alpha, tex.a);

    float Y = dot(result.rgb, primaryBrightness);
    result = vec4(mix(vec3(Y), result.rgb, saturation), result.a);
    result *= modulation;

    gl_FragColor = result;
}
