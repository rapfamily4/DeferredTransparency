#version 460 core


// --- Struct definitions
struct Material {
    vec4 diffuse;
    float roughness;
    float metalness;
    float ambientOcclusion;
};

// --- Render targets
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffuse;
layout (location = 3) out vec3 gRoughnessMetalnessAO;

// --- Input
in vec3 vPosition;
in vec3 vNormal;

// --- Uniforms
uniform bool executeDepthPeeling;
uniform bool firstPass;
uniform sampler2D previousDepth;
uniform sampler2D opaqueDepth;
uniform float bufferWidth;
uniform float bufferHeight;
uniform Material material;


// --- Main function
void main(void) {
    // Depth peeling
    if (executeDepthPeeling) {
        vec2 texCoord = vec2(float(gl_FragCoord.x) / bufferWidth, float(gl_FragCoord.y) / bufferHeight);
        
        // Peel depth layer
        if (!firstPass && gl_FragCoord.z <= texture(previousDepth, texCoord).r)
            discard;

        // Discard if covered by opaque fragment
        if (gl_FragCoord.z >= texture(opaqueDepth, texCoord).r)
            discard;
    }

    // Store fragment's position in view-space
    gPosition = vPosition;

    // Store fragment's normal in view-space
    gNormal = normalize(vNormal);

    // Store fragment's color
    gDiffuse = material.diffuse;

    // Store fragment's roughness
    gRoughnessMetalnessAO.r = material.roughness;

    // Store fragment's metalness
    gRoughnessMetalnessAO.g = material.metalness;

    // Store fragment's ambient occlusion
    gRoughnessMetalnessAO.b = material.ambientOcclusion;
}
