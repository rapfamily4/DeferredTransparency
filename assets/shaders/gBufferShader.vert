#version 460 core


// --- Attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

// --- Output
out vec3 vPosition;
out vec3 vNormal;

// --- Uniforms
// Matrices
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;


// --- Main function
void main() {    
    // Store vertex position in view-space
    vec4 vPosition4 = viewMatrix * modelMatrix * vec4(position, 1.0);
    vPosition = vPosition4.xyz;

    // Store normal in view-space
    vNormal = normalize(normalMatrix * normal);

    // Compute final position
    gl_Position = projectionMatrix * vPosition4;
}
