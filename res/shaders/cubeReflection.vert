#version 330 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec3 vNormal;
out vec3 vPosition;
out vec2 vTexCoord;

uniform mat4 uViewProjReflection;
uniform mat4 uModel;

void main() {

    vNormal = normalize(uModel * vec4(aNormal, 0)).xyz;
    vPosition = (uModel * aPos).xyz;
    vTexCoord = aTexCoord;
    gl_Position = uViewProjReflection * uModel * aPos;
}
