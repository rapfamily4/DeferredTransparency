// Written by Luigi Rapetta, 2022.

#ifndef MATERIAL_MANAGER_H
#define MATERIAL_MANAGER_H

#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rendering/material.hpp"
#include "scene/entity.hpp"
#include "consts.hpp"


// --- MaterialManager class
class MaterialManager {
public:
    // --- Public static methods
    static Material *newMaterial(std::string name, glm::vec4 diffuse = MATERIAL_DIFFUSE, float roughness = MATERIAL_ROUGHNESS, float metalness = MATERIAL_METALNESS, float ambientOcclusion = MATERIAL_AMBIENT_OCCLUSION);
    static Material *getMaterial(std::string name);
    static std::map<std::string, Material> *getMaterials();
    static void assignMaterial(Material *material, Entity *entity);
    static void updateAssignedEntities(Material *material);
    static void clear();

private:
    // --- Private constructor
    MaterialManager();

    // --- Private static members
    static std::map<std::string, Material> s_materials;
    static std::map<Material*, std::vector<Entity*>> s_materialAssignments;
};


#endif // MATERIAL_MANAGER_H