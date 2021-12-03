#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube prevSkybox;
uniform samplerCube currSkybox;
uniform float blendFactor;
uniform vec3 skyTint;

void main() {
    vec4 colorPrev = texture(prevSkybox, TexCoords);
    vec4 colorCurr = texture(currSkybox, TexCoords);
    // Mixing the previous sky box with the current skybox so it's a smooth
    // transition
    FragColor = mix(colorPrev, colorCurr, blendFactor);
}