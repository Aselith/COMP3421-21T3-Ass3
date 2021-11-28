

#version 330 core

#define MAX_LIGHTS 101

#define WAVE_STRENGTH 0.005
#define SHINE_FACTOR  16.0
#define REFLECTIVITY  0.5

in vec2 vTexCoord;
in vec4 glPositionSpace;
in vec3 vPosition;
in float rippleEffect;

out vec4 FragColor;

struct SpotLight {
    vec3 position;
    vec3 specular;
};

uniform float waterLevel;
uniform vec2 rippleCycle;
uniform vec2 uPitchFactor;
uniform vec3 sunColor;
uniform vec3 sunPos;
uniform vec3 uCameraPos;
uniform SpotLight allLights[MAX_LIGHTS];

uniform sampler2D uTex;
uniform sampler2D uReflection;
uniform sampler2D uRefraction;
uniform sampler2D uDuDvMap;
uniform sampler2D uNormalMap;

vec3 calcPointLight(SpotLight light, vec3 normal) {

    vec3 view = normalize(uCameraPos - vPosition);
    vec3 lightDir = normalize(light.position - vPosition);

    if (dot(lightDir, normal) <= 0 || light.position.y < waterLevel - 0.001f) {
        return vec3(0, 0, 0);
    }
   
    // Adjusted to use Bling Phong Model
    vec3 halfwayDir = normalize(lightDir + view);
    vec3 specular = light.specular * pow(max(dot(normal, halfwayDir), 0.0), SHINE_FACTOR);

    float distance = distance(light.position, vPosition);
    float attenuation = 1.0f / (1.0f + (0.2 * distance * distance));

    return specular * attenuation * REFLECTIVITY;
}

void main() {
    // Converting to projective co-ordinates
    vec2 seaReflectionCoord = (glPositionSpace.xy / glPositionSpace.w) / 2.0f + 0.5;
    vec2 seaRefractionCoord = seaReflectionCoord;

    // Calculating dudvmap
    float rippleEffect = rippleCycle.x / rippleCycle.y;

    vec2 distortTexCoords = texture(uDuDvMap, vec2(vTexCoord.x + rippleEffect, vTexCoord.y)).rg / 40.0f;
    distortTexCoords = vTexCoord + vec2(distortTexCoords.x, distortTexCoords.y + rippleEffect);
    distortTexCoords /= 2;
    vec2 totalRipple = (texture(uDuDvMap, distortTexCoords).rg * 2.0 - 1.0) * WAVE_STRENGTH;

    seaReflectionCoord += totalRipple;
    seaRefractionCoord += totalRipple;

    // Clamping
    seaReflectionCoord = clamp(seaReflectionCoord, 0.001f, 0.999f);
    seaRefractionCoord = clamp(seaRefractionCoord, 0.001f, 0.999f);

    // Flipping texture around along the y axis
    seaReflectionCoord.y *= -1;

    vec4 normalMapColor = texture(uNormalMap, distortTexCoords);
    vec3 normal = normalize(vec3(normalMapColor.r * 2.0 - 1.0, normalMapColor.b * 2.0 - 1.0, normalMapColor.g * 2.0 - 1.0));
    vec3 resultPointLightTotal = vec3(0, 0, 0);
    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (allLights[i].position.x >= 0 && allLights[i].position.y >= 0 && allLights[i].position.z >= 0) {
            resultPointLightTotal += calcPointLight(allLights[i], normal);
        }
    }

    // Calculating the fresnel effect
    float mixFactor = uPitchFactor.x / uPitchFactor.y;
    mixFactor = clamp(mixFactor, 0.15f, 0.85f);

    vec4 reflectTex = texture(uReflection, seaReflectionCoord);
    vec4 refractTex = texture(uRefraction, seaRefractionCoord);
    vec4 reflecRefrac = mix(reflectTex, refractTex, mixFactor);
    FragColor = mix(texture(uTex, vTexCoord), reflecRefrac, 0.20f) + vec4(resultPointLightTotal, 0.0);

}