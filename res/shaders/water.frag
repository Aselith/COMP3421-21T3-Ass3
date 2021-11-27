

#version 330 core

in vec2 vTexCoord;
in vec4 glPositionSpace;
in vec3 vNormal;
in vec3 vPosition;

out vec4 FragColor;

uniform sampler2D uTex;
uniform sampler2D uReflection;

void main() {
    // Converting to projective co-ordinates
    vec2 seaReflectionCoord = (glPositionSpace.xy / glPositionSpace.w) / 2.0f + 0.5;
    // Flipping texture around along the y axis
    seaReflectionCoord.y *= -1;
    FragColor = mix(texture(uTex, vTexCoord), texture(uReflection, seaReflectionCoord), 0.5);
}
