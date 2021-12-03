#version 330 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;
out vec4 glPositionSpace;
out vec3 vPosition;

uniform mat4 uViewProj;
uniform mat4 uModel;

void main() {
    vTexCoord = aTexCoord;
    vPosition = (uModel * aPos).xyz;
    gl_Position = uViewProj * uModel * aPos;

    glPositionSpace = gl_Position;
}
