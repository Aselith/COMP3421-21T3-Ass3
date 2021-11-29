#version 330 core
out vec4 FragColor;

in vec3 vNormal;
in vec3 vPosition;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main() {
    vec3 incidence = normalize(vPosition - cameraPos);
    vec3 reflection = reflect(incidence, normalize(vNormal));
    FragColor = vec4(texture(skybox, reflection).rgb, 1.0f);
}