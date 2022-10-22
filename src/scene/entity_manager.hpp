// Written by Luigi Rapetta, 2022.

#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <list>
#include <vector>

#include <glm/glm.hpp>

#include "consts.hpp"
#include "rendering/material_manager.hpp"
#include "resources/model.hpp"
#include "scene/entity.hpp"


// EntityManager class
class EntityManager {
    public:
        // --- Public static methods
        static Entity *newEntity(Model *model, Material *material, glm::vec3 position = ENTITY_POS, glm::vec3 rotation = ENTITY_ROT);
        static std::vector<Entity*> *getOpaqueEntities();
        static std::vector<Entity*> *getTransparentEntities();
        static void setEntityTransparency(Entity *entity, bool isTransparent);
        static void clear();

    private:
        // --- Private constructor
        EntityManager();

        // --- Private static members
        static std::list<Entity> s_entities;
        static std::vector<Entity*> s_opaqueEntities;
        static std::vector<Entity*> s_transparentEntities;
};


#endif // ENTITY_MANAGER_H