#version 460 core


// --- Struct definitions
// DON'T USE VEC3: https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct PointLight {
    vec4 position;
    vec4 color;
    vec4 constantLinearQuadratic;
};

// --- Shader Storage Buffers
layout(std430, binding = 0) buffer PointLights {
    PointLight pointLights[];
};

// --- Constants
const float PI = 3.14159265359;

// --- Input
in vec2 texCoords;

// --- Output
layout (location = 0) out vec4 opaqueBuffer;

// --- Uniforms
// G-buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gRoughnessMetalnessAO;
// Lights
uniform vec3 ambientLight;
uniform int pointLightsNumber;

// --- Subroutines' declarations
subroutine vec3 localModel(vec3 fragmentPos, vec3 N, vec3 diffuse, float roughness, float metalness);
subroutine uniform localModel LocalModel;

// --- Functions
float distributionGGX(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;
    
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return a2 / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0; // In IBL, k = (roughness*roughness)/2
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

float geometrySmith(float NdotV, float NdotL, float roughness) {
    float G1 = geometrySchlickGGX(NdotV, roughness);
    float G2 = geometrySchlickGGX(NdotL, roughness);
    return G1 * G2;
}

vec3 fresnelSchlick(float HdotV, vec3 F0) {
    // The Fresnel reflectance equation describes the ratio of light that gets reflected over the light that gets refracted.
    // F0 can be interpeted as the characteristic specular color of the substance.
    //
    // Clamp here to prevents black spots.
    return F0 + (1.0 - F0) * pow(clamp(1.0 - HdotV, 0.0, 1.0), 5.0);
}

// --- Subroutines
subroutine(localModel)
vec3 GGX(vec3 fragmentPos, vec3 N, vec3 diffuse, float roughness, float metalness) {
    // Compute V; keep N * V dot product
    vec3 V = normalize(-fragmentPos);
    float NdotV = max(dot(N, V), 0.0);

    // Calculate base reflectance F0
    //
    // F0 can be interpeted as the characteristic specular color of the substance.
    // The Fresnel equation used below is not thought for metallic surfaces. In order to use it with metals, a tinted F0 is used.
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, diffuse, metalness);

    // Calculate reflectance Lo
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < pointLightsNumber; i++) {
        // Retrieve light
        PointLight light = pointLights[i];
        // Light position has to be transformed into view-space from the application stage.
        // Fragment position is already in view-space.
        vec3 vPointLightDist = light.position.xyz - fragmentPos;
        
        // Compute H and L
        vec3 L = normalize(vPointLightDist);
        vec3 H = normalize(V + L);

        // Calculate radiance
        float distance = length(vPointLightDist);
        float attenuation = 1.0 / (light.constantLinearQuadratic.x + light.constantLinearQuadratic.y * distance + light.constantLinearQuadratic.z * (distance * distance));
        vec3 radiance = light.color.rgb * attenuation;

        // Keep dot products
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);

        // Cook-Torrance BRDF
        // Distribution of microfacets
        float D = distributionGGX(NdotH, roughness);
        // Geometry attenuation
        float G = geometrySmith(NdotV, NdotL, roughness);
        // Fresnel
        vec3 F = fresnelSchlick(HdotV, F0);

        // Compute the ratio of reflected light over refracted light
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metalness; // Metallic surfaces show no diffuse colors

        // Compute lambert component
        vec3 lambert = kD * diffuse / PI;

        // Compute specular component
        vec3 numerator = D * G * F;
        float denominator = 4.0 * NdotL * NdotV + 0.0001; // Add small constant to prevent division by zero
        vec3 specular = numerator / denominator;

        // Rendering equation
        Lo += (lambert + specular) * radiance * NdotL;
    }

    // Return
    return Lo;
}


// --- Main function
void main(void) {    
    // Fetch surface data from G-buffer textures
    vec4 diffuse = texture(gDiffuse, texCoords);
    if (diffuse.a <= 0.0001) discard;
    vec3 vPosition = texture(gPosition, texCoords).rgb;
    vec3 vNormal = texture(gNormal, texCoords).rgb;
    float roughness = texture(gRoughnessMetalnessAO, texCoords).r;
    float metalness = texture(gRoughnessMetalnessAO, texCoords).g;
    float ambientOcclusion = texture(gRoughnessMetalnessAO, texCoords).b;

    // Initialize color
    vec3 color = vec3(0.0);

    // Add ambient light
    color += ambientLight * diffuse.rgb * ambientOcclusion;
    
    // Run local illumination model
    color += LocalModel(vPosition, vNormal, diffuse.rgb, roughness, metalness);

    // Multiply rbg component by alpha in order to compensate the missing multiply by Asrc
    //
    // The front-to-back blending equation is:
    //      Cdst = Adst (Asrc Csrc) + Cdst
    //      Adst = (1-Asrc) Adst
    // The blending options set back in the application produce:
    //      Cdst = Adst (Csrc) + Cdst
    //      Adst = (1-Asrc) Adst
    // The missing multiply by Asrc must be compensated in this shader, by simply multiplying
    // the rgb component of the final color with the alpha value.
    //
    // SOURCE: https://community.khronos.org/t/front-to-back-blending/65155/3
    color *= diffuse.a;

    // Store fragment's color
    opaqueBuffer = vec4(color, diffuse.a);
}
