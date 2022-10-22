// Written by Luigi Rapetta, 2022.

#ifndef LIGHTS_H
#define LIGHTS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// --- Light data structures
struct PointLight {
    glm::vec4 position;
    glm::vec4 color;
    glm::vec4 constantLinearQuadratic;
};

/*
    Don't use the following:

    struct PointLight {
        glm::vec3 position;
        glm::vec3 color;
        float constant;
        float linear;
        float quadratic;
    };


    The reason: there's a problematic discrepancy between data alignment rules of C++
    compilers and GLSL compilers when 3-dimensional data structures are concerned.

    In other words, 3-dimensional data structures are interpreted in different ways
    between C++ and GLSL. This can cause serious issues when loading application-side
    data to the GPU.

    SOURCE: https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
*/

#endif // LIGHTS_H