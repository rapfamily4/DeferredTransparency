#version 460 core


// --- Attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 textureCoords;

// --- Output
out vec2 texCoords;


// --- Main function
void main() {
    texCoords = textureCoords;
    gl_Position = vec4(position, 1.0);
}
