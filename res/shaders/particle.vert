#version 330 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

uniform mat4 uProj;
uniform mat4 uViewModel;
uniform mat4 uModel;
uniform mat4 uLightProj;

uniform vec4 plane;

void main() {
    gl_ClipDistance[0] = dot(uModel * aPos, plane);
    vTexCoord = aTexCoord;
    gl_Position = uProj * uViewModel * aPos;
}
