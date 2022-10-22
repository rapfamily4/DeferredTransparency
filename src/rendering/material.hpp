// Written by Luigi Rapetta, 2022.

#ifndef MATERIAL_H
#define MATERIAL_H

#include <stdlib.h>
#include <string>

#include <glm/glm.hpp>

#include "consts.hpp"


// --- Material struct
struct Material {
    glm::vec4 diffuse;
    float roughness;
    float metalness;
    float ambientOcclusion;
};


#endif // MATERIAL_H