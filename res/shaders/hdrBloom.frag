#version 330 core

out vec4 FragColor;
in vec2 vTexCoord;

uniform sampler2D scene;
uniform sampler2D bloomBlur;

#define GAMMA 2.2

uniform int hdr;
uniform float exposure;
uniform bool isUnderwater;
uniform float cycle;


// 0 - off
// 1 - Exposure
// 2 - Filmic
// 3 - Reinhard

vec3 tonemapExposure(vec3 color) {
    vec3 result = vec3(1.0) - exp(-color * exposure);
    return pow(result, vec3(GAMMA));
}

vec3 tonemapFilmic(vec3 color) {
    color = max(vec3(0.0), color - 0.004);
    vec3 result = (color * (6.2 * color + 0.5)) / (color * (6.2 * color + 1.7) + 0.06);
    return pow(result, vec3(GAMMA));
}

vec3 tonemapReinhard(vec3 color) {
    vec3 result = color / (color + vec3(1.0));
    return result;
}

vec3 tonemapLotte(vec3 color) {
    // Attribute: https://github.com/dmnsgn/glsl-tone-map/blob/master/lottes.glsl
    const vec3 a = vec3(1.6);
    const vec3 d = vec3(0.977);
    const vec3 hdrMax = vec3(8.0);
    const vec3 midIn = vec3(0.18);
    const vec3 midOut = vec3(0.267);

    const vec3 b =
        (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
    const vec3 c =
        (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

    return pow(color, a) / (pow(color, a * d) * b + c);
}

vec3 tonemapUnreal(vec3 color) {
    // Attribute: https://github.com/dmnsgn/glsl-tone-map/blob/master/unreal.glsl
    return pow(color / (color + 0.155) * 1.019, vec3(GAMMA));
}

vec3 tonemapNeg(vec3 color) {
    return vec3(1.0 - color.r, 1.0 - color.g, 1.0 - color.b);
}

void main() {
    vec3 hdrColor, bloomColor;
    float value = 12 * (vTexCoord.y - cycle);
    if (isUnderwater) {

        vec2 newTexCoord = vTexCoord;
        newTexCoord.x = newTexCoord.x + 0.01 * sin(value);
        newTexCoord = clamp(newTexCoord, 0.001, 0.999);

        hdrColor = texture(scene, newTexCoord).rgb;      
        bloomColor = texture(bloomBlur, newTexCoord).rgb * 2;
    } else {
        hdrColor = texture(scene, vTexCoord).rgb;      
        bloomColor = texture(bloomBlur, vTexCoord).rgb;
    }
    
    if (hdr != 0) {
        // Apply the full bloom color if hdr is not set to off
        hdrColor += bloomColor;
    } else {
        hdrColor += bloomColor / 2;
    }
    
    vec3 result = hdrColor;

    if (isUnderwater) {
        result = mix(vec4(result, 1.0), vec4(49.0 / 255.0, 83.0 / 255.0, 152.0 / 255.0, 1.0), 0.5).rgb;
    }

    // tone mapping
    switch (hdr) {
        case 1:
            result = tonemapExposure(result);
            break;
        case 2:
            result = tonemapFilmic(result);
            break;
        case 3:
            result = tonemapReinhard(result);
            break;
        case 4:
            result = tonemapLotte(result);
            break;
        case 5:
            result = tonemapUnreal(result);
            break;
        case 6:
            result = tonemapNeg(result);
            break;
    }
    FragColor = vec4(result, 1.0);
    
}