#version 330 core

out vec4 FragColor;
in vec2 vTexCoord;

uniform sampler2D scene;
uniform sampler2D bloomBlur;

uniform bool hdr;
uniform bool isBlur;
uniform float exposure;

void main() {

    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, vTexCoord).rgb;      
    vec3 bloomColor = texture(bloomBlur, vTexCoord).rgb;

    if (isBlur) {
        hdrColor += bloomColor;
    }
    
    vec3 result = hdrColor;
    // tone mapping
    if (hdr) {
        result = vec3(1.0) - exp(-hdrColor * exposure);
        result = pow(result, vec3(gamma));
    }

    FragColor = vec4(result, 1.0);
}