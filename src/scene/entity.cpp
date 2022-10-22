// Written by Luigi Rapetta, 2022.

#include <glm/glm.hpp>

#include "scene/entity.hpp"
#include "rendering/material_manager.hpp"


// --- Public constructor
Entity::Entity(Model *model, Material *material, glm::vec3 position, glm::vec3 rotation) :
    m_model{ model },
    m_material{ material },
    m_position{ position },
    m_rotation{ rotation } { /* --- */ }

// --- Public methods
Model *Entity::getModel() {
    return m_model;
}

Material *Entity::getMaterial() {
    return m_material;
}

glm::vec3 Entity::getPosition() {
    return m_position;
}

glm::vec3 Entity::getRotation() {
    return m_rotation;
}

void Entity::setMaterial(Material *material) {
    if (material == NULL) return;
    MaterialManager::assignMaterial(material, this);
    m_material = material;
}