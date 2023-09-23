#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "context_manager.hpp"
#include "input/input_manager.hpp"
#include "rendering/light_manager.hpp"
#include "rendering/material_manager.hpp"
#include "rendering/renderer.hpp"
#include "resources/resource_manager.hpp"
#include "resources/model.hpp"
#include "scene/entity_manager.hpp"


// --- Utility functions
float getRandom(float min, float max) {
    float random = float(rand())/float((RAND_MAX));
    float newRange = max - min;
    random = random * newRange + min;
    return random;
}

float convertToRange(float number, float oldMin, float oldMax, float newMin, float newMax) {
    float oldRange = oldMax - oldMin;
    if (oldRange == 0.f) return newMin;
    else {
        float newRange = newMax - newMin;
        return (((number - oldMin) * newRange) / oldRange) + newMin;
    }
}


// --- Main loop
int main(int argc, char** argv) {
    // Initialize
    ContextManager::init("OpenGL 4.6 Deferrer Renderer");
    srand((unsigned int)time(NULL));
    
    // Load model
    Model *model = ResourceManager::loadModel("assets/models/teapot.obj");
    Model *background = ResourceManager::loadModel("assets/models/background_cube.obj");

    // Generate point lights
    float lightsExtent = 10.f; // They'll clip out the background cube, but that's not a big issue...
    for (int i = 0; i < LIGHT_MAXSHOWN; i++) {
        glm::vec3 randomPosition =  glm::vec3{ getRandom(-lightsExtent, lightsExtent), getRandom(-lightsExtent, lightsExtent), getRandom(-lightsExtent, lightsExtent) };
        glm::vec3 randomColor = glm::vec3{ getRandom(0.0f, 1.0f), getRandom(0.0f, 1.0f), getRandom(0.0f, 1.0f) };
        LightManager::newPointLight(randomPosition, randomColor);
    }

    // Generate materials
    Material *materials[4];
    materials[0] = MaterialManager::newMaterial("Material 0", glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}, 0.25f, 0.f);
    materials[1] = MaterialManager::newMaterial("Material 1", glm::vec4{0.0f, 1.0f, 0.0f, 0.5f}, 0.25f, 0.f);
    materials[2] = MaterialManager::newMaterial("Material 2", glm::vec4{0.75f, 0.35f, 1.0f, 0.5f}, 0.25f, 1.f);
    materials[3] = MaterialManager::newMaterial("Material 3", glm::vec4{0.75f, 0.35f, 0.0f, 1.0f}, 0.25f, 1.f);
    Material *matBackground = MaterialManager::newMaterial("Material Background", glm::vec4{0.75f, 0.75f, 0.75f, 1.0f}, 0.45f, 1.0f);

    // Generate entities
    float entitiesExtent = 5.f;
    for (int i = 0; i < 25; i++) {
        float x = convertToRange((float)(i % 5), 0.f, 4.f, -entitiesExtent, +entitiesExtent);
        float y = convertToRange((float)(i / 5), 0.f, 4.f, -entitiesExtent, +entitiesExtent);
        EntityManager::newEntity(model, materials[0], glm::vec3{ x, y, -8.0f });
        EntityManager::newEntity(model, materials[1], glm::vec3{ x, y, -4.0f });
        EntityManager::newEntity(model, materials[2], glm::vec3{ x, y, +4.0f });
        EntityManager::newEntity(model, materials[3], glm::vec3{ x, y, +8.0f });
    }
    EntityManager::newEntity(background, matBackground);

    // Main rendering loop
    while (!ContextManager::shouldClose()) {
        // Prepare for next rendering cycle
        ContextManager::next();

        // Update lights
        std::vector<PointLight> *pointLights = LightManager::getPointLights();
        unsigned int pointLightsSize = pointLights->size();
        for (int i = 0; i < pointLightsSize; i++) {
            glm::mat4 model{1.0f};
            model = glm::rotate(model, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(LightManager::getPointLightsRotationSpeed()) * ContextManager::getDeltaTime(), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
            (*pointLights)[i].position = model * (*pointLights)[i].position;
        }
        LightManager::updatePointLightsSSBO(Renderer::getCamera().getViewMatrix());

        // Render
        unsigned int pointLightsSSBO = LightManager::getPointLightsSSBO();
        unsigned int shownPointLightsSize = (unsigned int)LightManager::getNumberOfShownPointLights();
        std::vector<Entity*> *opaqueEntities = EntityManager::getOpaqueEntities();
        std::vector<Entity*> *transparentEntities = EntityManager::getTransparentEntities();
        glm::vec3 ambientLight = LightManager::getAmbientLight();
        Renderer::renderEntities(opaqueEntities, transparentEntities, ambientLight, pointLightsSSBO, shownPointLightsSize);
        Renderer::renderOnDefaultFramebuffer();

        // Dear ImGui
        ContextManager::displayGUI();
    }
    
    // Clear resources
    ResourceManager::clear();
    MaterialManager::clear(); // <-- A bug is needed to be solved here (small sized oofy, though)
    LightManager::clear();
    EntityManager::clear();
    Renderer::clear();
    ContextManager::clear();
    
    // Return
    return 0;
}