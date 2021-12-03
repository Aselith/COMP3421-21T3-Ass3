/*
#version 330 core

uniform mat4 uMVP;

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;


void main() {
    vTexCoord = aTexCoord;
    gl_Position = uMVP * aPos;
}
*/
#version 330 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vPosition;
out vec4 fragPosLightSpace;

uniform mat4 uViewProj;
uniform mat4 uModel;
uniform mat4 uLightProj;
uniform vec4 plane;

void main() {
    vTexCoord = aTexCoord;
    if (aNormal.x == 0 && aNormal.y == 0 && aNormal.z == 0) {
        vNormal = aNormal;
    } else {
        vNormal = normalize(uModel * vec4(aNormal, 0)).xyz;
    }
    vPosition = (uModel * aPos).xyz;

    gl_ClipDistance[0] = dot(uModel * aPos, plane);

    // Shadow shenanigans
    fragPosLightSpace = uLightProj * vec4(vPosition, 1.0);

    gl_Position = uViewProj * uModel * aPos;
}
