/*
#version 330 core

in vec2 vTexCoord;

uniform sampler2D uTex;
uniform float uTexFactor;
uniform vec4 uColor;

out vec4 fFragColor;

void main() {
    fFragColor = mix(uColor, texture(uTex, vTexCoord), uTexFactor);
}
*/

#version 330 core

#define MAX_LIGHTS 101

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vPosition;
in vec4 fragPosLightSpace;
out vec4 fFragColor;

uniform sampler2D uTex;
uniform sampler2D uSpec;
uniform sampler2D uDepthMap;

struct Material {
    float texFactor;
    float specularFactor;
    vec3 ambient;
    vec4 color;
    vec4 specular;
    vec3 diffuse;
    float phongExp;
};

struct DirLight {
    vec3 direction;
    vec3 color;
    float ambient;
};

struct SpotLight {
    vec3 position;
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;
    float intensity;
};

uniform Material uMat;
uniform DirLight uSun;
uniform vec3 uCameraPos;
uniform SpotLight allLights[MAX_LIGHTS];
uniform bool isIlluminating;
uniform bool affectedByShadows;
uniform bool forceBlack;

vec3 rgbToLinear(vec3 col) {
    return pow(col, vec3(2.2));
}

vec3 linearToRgb(vec3 col) {
    return pow(col, vec3(1/2.2));
}

float calcShadow() {
    if (!affectedByShadows) {
        return 1.0f;
    }
    vec3 pos = fragPosLightSpace.xyz * 0.5 + 0.5;
    pos.z = pos.z > 1.0 ? 1.0 : pos.z;
    
    // Bias correcting
    float biasLight = max(0.0109 * (1.0 - dot(-uSun.direction, vNormal)), 0.005);

    // PCF
    float returnShadowValue = 0.0f;
    vec2 texelSize = 1.0f / textureSize(uDepthMap, 0);
    float totalValues = 0.0f, sampleSize = 1.0f;
    
    for (float x = -sampleSize; x <= sampleSize; x++) {
        for (float y = -sampleSize; y <= sampleSize; y++) {
            totalValues++;
            float depth = texture(uDepthMap, pos.xy + vec2(x, y) * texelSize).r;
            returnShadowValue += (depth + biasLight) < pos.z ? 0.0f : 1.0f;
        }
    }
    return returnShadowValue / totalValues;
}

vec3 calcPointLight(SpotLight light, vec3 mat_diffuse, vec3 mat_specular) {
    vec3 ambient = light.ambient * mat_diffuse * uMat.ambient;
    vec3 lightDir = normalize(light.position - vPosition);
    vec3 diffuse = light.diffuse * mat_diffuse * max(0,dot(lightDir, vNormal));
    
    vec3 view = normalize(uCameraPos - vPosition);
   
    // Adjusted to use Bling Phong Model
    vec3 halfwayDir = normalize(lightDir + view);
    vec3 specular = light.specular * mat_specular * pow(max(dot(vNormal, halfwayDir), 0), uMat.phongExp);
    // Old code using original phong model
    // vec3 reflected = reflect(-lightDir, vNormal);
    // vec3 specular = light.specular * mat_specular * pow(max(0, dot(reflected, view)), uMat.phongExp);

    float distance = distance(light.position, vPosition);
    float attenuation = 1.0f / (1.0f + (0.2 * distance * distance));

    return (ambient + diffuse + specular) * attenuation * light.intensity;
}

void main() {
    
    if (vNormal.x == 0 && vNormal.y == 0 && vNormal.z == 0) {
        fFragColor = texture(uTex, vTexCoord);
        if (isIlluminating) {
            fFragColor *= vec4(1.5, 1.5, 1.5, 1.0);
        }
    } else {
        // Calculating diffuse by lighting
        vec4 color = mix(uMat.color, texture(uTex, vTexCoord), uMat.texFactor);
        if (forceBlack) color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        color.rgb = rgbToLinear(color.rgb);
        
        vec3 ambient = rgbToLinear(uSun.color) * pow(uSun.ambient, 2.2);
        float lightNormal = dot(-uSun.direction, vNormal);
        vec3 diffuse = rgbToLinear(uSun.color) * rgbToLinear(uMat.diffuse) * max(0, lightNormal) * 1.1f;

        // Calculating specular
        vec4 mat_specular = mix(uMat.specular, texture(uSpec, vTexCoord), uMat.specularFactor);
        vec3 mat_specularV3 = rgbToLinear(mat_specular.rgb);

        // Only calculate spot light if there is a diffuse map. This is to avoid lighting on
        // the sky box
        vec3 resultPointLightTotal;
        if (uMat.texFactor == 1.0f) {
            for (int i = 0; i < MAX_LIGHTS; i++) {
                if (allLights[i].position.x >= 0 && allLights[i].position.y >= 0 && allLights[i].position.z >= 0) {
                    resultPointLightTotal += rgbToLinear(calcPointLight(allLights[i], color.rgb, mat_specularV3));
                }
            }
            diffuse = diffuse * calcShadow(); // Decides if the fragment is within a shadow
        }
        fFragColor = vec4(linearToRgb((ambient + diffuse + resultPointLightTotal) * color.rgb), color.a);
    }
}
