

#version 330 core

in vec2 vTexCoord;

out vec4 FragColor;

uniform sampler2D uTex;
uniform vec3 avgColor; 

void main() {
    float lumin = 0.2126f * avgColor.r + 0.7152f * avgColor.g + 0.0722f * avgColor.b;
    lumin = clamp(lumin * 1.2f, 0.0f, 1.0f);
    FragColor = vec4(texture(uTex, vTexCoord).rgb, 1.0f);
    FragColor = mix(vec4(0.0f, 0.0f, 0.0f, 1.0f), FragColor, lumin);
}