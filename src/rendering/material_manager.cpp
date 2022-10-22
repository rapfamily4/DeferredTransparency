#include <cmath>
#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rendering/material_manager.hpp"
#include "scene/entity_manager.hpp"
#include "consts.hpp"


// --- Public static members
std::map<std::string, Material> MaterialManager::s_materials;
std::map<Material*, std::vector<Entity*>> MaterialManager::s_materialAssignments;


// --- Public static functions
Material *MaterialManager::newMaterial(std::string name, glm::vec4 diffuse, float roughness, float metalness, float ambientOcclusion) {
    s_materials[name] = Material{ diffuse, roughness, metalness, ambientOcclusion };
    s_materialAssignments[&s_materials[name]] = std::vector<Entity*>{};
    return &s_materials[name];
}

Material *MaterialManager::getMaterial(std::string name) {
    return &s_materials[name];
}

std::map<std::string, Material> *MaterialManager::getMaterials() {
    return &s_materials;
}

void MaterialManager::assignMaterial(Material *material, Entity *entity) {
    // Remove existing assignment of material to this entity
    bool found = false;
    for (auto mapIter = s_materialAssignments.begin(); !found && mapIter != s_materialAssignments.end(); ++mapIter)
        for (auto vecIter = mapIter->second.begin(); !found && vecIter != mapIter->second.end(); ++vecIter) {
            if ((*vecIter) == entity) {
                found = true;
                mapIter->second.erase(vecIter);
            }
        }

    // Assign this material to this entity
    s_materialAssignments[material].push_back(entity);
}

void MaterialManager::updateAssignedEntities(Material *material) {
    std::vector<Entity*> assignments = s_materialAssignments[material]; // Not very safe
    for (auto iter = assignments.begin(); iter != assignments.end(); ++iter)
        EntityManager::setEntityTransparency(*iter, material->diffuse.a < 1.f);
}

void MaterialManager::clear() {
    // TO SOLVE: clearing s_materials won't work, as well as deleting its strings and materials directly.
    //s_materials.clear();
    
    for (auto mapIter = s_materialAssignments.begin(); mapIter != s_materialAssignments.end(); ++mapIter) {
        mapIter->second.clear();
    }
    s_materialAssignments.clear();
}