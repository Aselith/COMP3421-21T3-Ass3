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

vec3 kernalFilter(float kernal[9]) {

    vec3 color = vec3(0.0f);

    for (int i = 0; i < 9; i++) {
        color += vec3(texture(uTex, vTexCoord.st + offsets[i])) * kernal[i];
    }
    return color;
}

vec3 neg(vec3 color) {
    return vec3(1.0 - color.r, 1.0 - color.g, 1.0 - color.b);
}

vec3 pixelate() {
    float width = 2560.0f;
    float height = 1440.0f;
    float dx = 10.0f * (1.0 / width);
    float dy = 10.0f * (1.0 / height);
    vec2 newTexCoord = vec2(dx * floor(vTexCoord.x / dx), dy * floor(vTexCoord.y / dy));
    return texture(uTex, newTexCoord).rgb;
}

void main() {
    vec3 result;

    switch(kernelType) {
        case 1:
            float kernelEdge[9] = float[]
            (
                1, 1, 1,
                1,-8, 1,
                1, 1, 1
            );
            result = kernalFilter(kernelEdge);
            break;
        case 2:
            float emboss[9] = float[]
            (
                -2,-1, 0,
                -1, 1, 1,
                0, 1, 2
            );
            result = kernalFilter(emboss);
            break;
        case 3:
            float sharpen[9] = float[]
            (
                0,-1, 0,
                -1, 5,-1,
                0,-1, 0
            );
            result = kernalFilter(sharpen);
            break;
        case 4:
            float rightSobel[9] = float[]
            (
                -1, 0, 1,
                -2, 0, 2,
                -1, 0, 1
            );
            result = kernalFilter(rightSobel);
            break;
        case 5:
            float blur[9] = float[]
            (
                0.0625, 0.125, 0.0625,
                0.1250, 0.250, 0.1250,
                0.0625, 0.125, 0.0625
            );
            result = kernalFilter(blur);
            break;
        case 6:
            result = pixelate();
            break;
        case 7:
            result = texture(uTex, vTexCoord).rgb;
            result = neg(result);
            break;
        default:
            result = texture(uTex, vTexCoord).rgb;
            break;
    }

    FragColor = vec4(result, 1.0f);

    if (isUnderwater) {
        vec4 twoFrameMerge;
        if (kernelType == 7) {
            twoFrameMerge = mix(vec4(neg(texture(uTempA, vTexCoord).rgb), 1.0f), vec4(neg(texture(uTempB, vTexCoord).rgb), 1.0), 0.85f);
        } else {
            twoFrameMerge = mix(texture(uTempA, vTexCoord), texture(uTempB, vTexCoord), 0.85f);
        }
        FragColor = mix(FragColor, twoFrameMerge, 0.75f);
    }
}