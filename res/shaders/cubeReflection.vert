#version 330 core

layout (location = 0) in vec4 aPos;
// layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec3 vNormal;
out vec3 vPosition;

uniform mat4 uViewProj;
uniform mat4 uModel;

void main() {

    vNormal = normalize(uModel * vec4(aNormal, 0)).xyz;
    vPosition = (uModel * aPos).xyz;

    gl_Position = uViewProj * uModel * aPos;
}
