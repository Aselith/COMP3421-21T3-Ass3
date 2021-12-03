#version 330 core
out vec4 FragColor;

in vec2 vTexCoord;

uniform sampler2D uTex;
uniform sampler2D uTempA;
uniform sampler2D uTempB;
uniform bool isUnderwater;
uniform int kernelType;

const float OFFSET_X = 1.0f / 800.0f;
const float OFFSET_Y = 1.0f / 800.0f;

vec2 offsets[9] = vec2[]
(
    vec2(-OFFSET_X, OFFSET_Y), vec2(0.0f, OFFSET_Y), vec2(OFFSET_X, OFFSET_Y),
    vec2(-OFFSET_X, 0.0f),     vec2(0.0f, 0.0f),     vec2(OFFSET_X, 0.0f),
    vec2(-OFFSET_X, -OFFSET_Y), vec2(0.0f, -OFFSET_Y), vec2(OFFSET_X, -OFFSET_Y)
);

vec3 kernalFilter(float kernal[9], sampler2D textureID) {

    vec3 color = vec3(0.0f);

    for (int i = 0; i < 9; i++) {
        color += vec3(texture(textureID, vTexCoord.st + offsets[i])) * kernal[i];
    }
    return color;
}

vec3 neg(vec3 color) {
    return vec3(1.0 - color.r, 1.0 - color.g, 1.0 - color.b);
}

vec3 pixelate(sampler2D textureID) {
    float width = 2560.0f;
    float height = 1440.0f;
    float dx = 10.0f * (1.0 / width);
    float dy = 10.0f * (1.0 / height);
    vec2 newTexCoord = vec2(dx * floor(vTexCoord.x / dx), dy * floor(vTexCoord.y / dy));
    return texture(textureID, newTexCoord).rgb;
}

void main() {
    vec3 result, frameA, frameB;

    switch(kernelType) {
        case 1:
            float kernelEdge[9] = float[]
            (
                1, 1, 1,
                1,-8, 1,
                1, 1, 1
            );
            result = kernalFilter(kernelEdge, uTex);
            frameA = kernalFilter(kernelEdge, uTempA);
            frameB = kernalFilter(kernelEdge, uTempB);
            break;
        case 2:
            float emboss[9] = float[]
            (
                -2,-1, 0,
                -1, 1, 1,
                 0, 1, 2
            );
            result = kernalFilter(emboss, uTex);
            frameA = kernalFilter(emboss, uTempA);
            frameB = kernalFilter(emboss, uTempB);
            break;
        case 3:
            float sharpen[9] = float[]
            (
                 0,-1, 0,
                -1, 5,-1,
                 0,-1, 0
            );
            result = kernalFilter(sharpen, uTex);
            frameA = kernalFilter(sharpen, uTempA);
            frameB = kernalFilter(sharpen, uTempB);
            break;
        case 4:
            float rightSobel[9] = float[]
            (
                -1, 0, 1,
                -2, 0, 2,
                -1, 0, 1
            );
            result = kernalFilter(rightSobel, uTex);
            frameA = kernalFilter(rightSobel, uTempA);
            frameB = kernalFilter(rightSobel, uTempB);
            break;
        case 5:
            float blur[9] = float[]
            (
                0.0625, 0.125, 0.0625,
                0.1250, 0.250, 0.1250,
                0.0625, 0.125, 0.0625
            );
            result = kernalFilter(blur, uTex);
            frameA = kernalFilter(blur, uTempA);
            frameB = kernalFilter(blur, uTempB);
            break;
        case 6:
            result = pixelate(uTex);
            frameA = pixelate(uTempA);
            frameB = pixelate(uTempB);
            break;
        case 7:
            result = texture(uTex, vTexCoord).rgb;
            frameA = texture(uTempA, vTexCoord).rgb;
            frameB = texture(uTempB, vTexCoord).rgb;
            result = neg(result);
            frameA = neg(frameA);
            frameB = neg(frameB);
            break;
        default:
            result = texture(uTex, vTexCoord).rgb;
            frameA = texture(uTempA, vTexCoord).rgb;
            frameB = texture(uTempB, vTexCoord).rgb;
            break;
    }

    FragColor = vec4(result, 1.0f);

    if (isUnderwater) {
        vec4 twoFrameMerge;
        twoFrameMerge = mix(vec4(frameA, 1.0f), vec4(frameB, 1.0f), 0.5f);
        FragColor = mix(FragColor, twoFrameMerge, 0.90f);
    }
}