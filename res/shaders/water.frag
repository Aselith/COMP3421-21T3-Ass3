

#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vPosition;

out vec4 FragColor;

uniform sampler2D uTex;

void main() {
    FragColor = texture(uTex, vTexCoord);
}
