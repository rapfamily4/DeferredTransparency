#include <glm/glm.hpp>

#include "rendering/material_manager.hpp"
#include "scene/entity_manager.hpp"


// --- Public static members
std::list<Entity> EntityManager::s_entities;
std::vector<Entity*> EntityManager::s_opaqueEntities{};
std::vector<Entity*> EntityManager::s_transparentEntities{};


// --- Public static functions
Entity *EntityManager::newEntity(Model *model, Material *material, glm::vec3 position, glm::vec3 rotation) {
    Entity entity = Entity{model, material, position, rotation};
    s_entities.push_back(entity);
    Entity *entPointer = &(s_entities.back());
    if (material->diffuse.a < 1.f) s_transparentEntities.push_back(entPointer);
    else s_opaqueEntities.push_back(entPointer);
    entPointer->setMaterial(material);
    return entPointer;
}

std::vector<Entity*> *EntityManager::getOpaqueEntities() {
    return &s_opaqueEntities;
}

std::vector<Entity*> *EntityManager::getTransparentEntities() {
    return &s_transparentEntities;
}

void EntityManager::setEntityTransparency(Entity *entity, bool isTransparent) {
    // Remove entity from both transparent and opaque vectors; clean and safe approach.
    bool found = false;
    for (auto iter = s_opaqueEntities.begin(); !found && iter != s_opaqueEntities.end(); ++iter)
        if ((*iter) == entity) {
            found = true;
            s_opaqueEntities.erase(iter);
        }
    found = false;
    for (auto iter = s_transparentEntities.begin(); !found && iter != s_transparentEntities.end(); ++iter)
        if ((*iter) == entity) {
            found = true;
            s_transparentEntities.erase(iter);
        }
    
    // Add entity to correct vector
    if (isTransparent) s_transparentEntities.push_back(entity);
    else s_opaqueEntities.push_back(entity);

}

void EntityManager::clear() {
    s_entities.clear();
    s_opaqueEntities.clear();
    s_transparentEntities.clear();
}