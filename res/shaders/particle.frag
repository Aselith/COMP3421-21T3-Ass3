

#version 330 core

in vec2 vTexCoord;

out vec4 FragColor;

struct DirLight {
    vec3 direction;
    vec3 ambient;
};

uniform sampler2D uTex;
uniform bool forceBlack;
uniform bool affectedByLight;
uniform DirLight uSun;


void main() {
    vec4 test = texture(uTex, vTexCoord);
    if (test.a == 0) {
        discard;
    }
    if (forceBlack) {
        FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    } else {
        vec4 baseColor = texture(uTex, vTexCoord);
        if (affectedByLight) {
            float lightNormal = max(0.1f, dot(uSun.direction, vec3(0, 1, 0)));
            vec4 lightColor = mix(vec4(uSun.ambient, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), 0.85f);
            FragColor = mix(baseColor, vec4(lightColor.rgb, baseColor.a), lightNormal * 0.8f);
        } else {
            FragColor = baseColor;
        }
    }
}