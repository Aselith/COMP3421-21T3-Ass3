#version 330 core
out vec4 FragColor;

in vec2 vTexCoord;

uniform sampler2D bloomTex;

void main() {
    FragColor = texture(bloomTex, vTexCoord);
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        FragColor = vec4(FragColor.rgb, 1.0);
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}