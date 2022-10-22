#version 460 core


// --- Input
in vec2 texCoords;

// --- Output
out vec4 fragColor;

// --- Uniforms
uniform sampler2D opaqueBuffer;
uniform sampler2D depthBuffer;


// --- Main function
void main(void) {
    // Fetch color
    vec3 color = texture(opaqueBuffer, texCoords).rgb;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // Gamma correction
    color = pow(color, vec3(1.0/2.2)); 

    // Store final color
    //float depth = texture(depthBuffer, texCoords).r;
    //color = vec3(depth);
    fragColor = vec4(color, 1.0);
}
