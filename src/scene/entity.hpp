// Written by Luigi Rapetta, 2022.

#ifndef ENTITY_H
#define ENTITY_H

#include <glm/glm.hpp>

#include "consts.hpp"
#include "rendering/material.hpp"
#include "resources/model.hpp"


// Entity
class Entity {
    public:
        // --- Public constructor
        Entity(Model *model, Material *material, glm::vec3 position = ENTITY_POS, glm::vec3 rotation = ENTITY_ROT);

        // --- Public methods
        Model *getModel();
        Material *getMaterial();
        glm::vec3 getPosition();
        glm::vec3 getRotation();
        void setMaterial(Material *material);

    private:
        // --- Private members
        Model *m_model;
        Material *m_material;
        glm::vec3 m_position;
        glm::vec3 m_rotation;
};


#endif // ENTITY_H