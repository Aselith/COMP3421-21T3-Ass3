#version 330 core
out vec4 FragColor;

in vec3 vNormal;
in vec3 vPosition;
in vec2 vTexCoord;

uniform vec3 cameraPos;
uniform sampler2D uTex;
uniform samplerCube environmentMap;

void main() {
    FragColor = texture(uTex, vTexCoord);
    if (FragColor.r == 0.0f && FragColor.g == 1.0f && FragColor.b == 0.0f) {
        // Only replace color if the color is 100% green
        vec3 incidence = normalize(vPosition - cameraPos);
        vec3 reflection = reflect(incidence, normalize(vNormal));
        FragColor = vec4(texture(environmentMap, reflection).rgb, 1.0f);
    }
}