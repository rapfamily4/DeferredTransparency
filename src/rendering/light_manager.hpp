// Written by Luigi Rapetta, 2022.

#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "consts.hpp"
#include "rendering/lights.hpp"


// LightManager class
class LightManager {
public:
    // --- Public static methods
    static void init();
    static PointLight *newPointLight(glm::vec3 position = LIGHT_POS, glm::vec3 diffuse = LIGHT_COLOR, float constant = LIGHT_CONSTANT, float linear = LIGHT_LINEAR, float quadratic = LIGHT_QUADRATIC);
    static void updatePointLightsSSBO(const glm::mat4 &viewMatrix, bool updateNotShown = false);
    static void setAmbientLight(glm::vec3 ambientLight);
    static void setNumberOfShownPointLights(int numberOfShown);
    static void setPointLightsRotationSpeed(float speed);
    static glm::vec3 getAmbientLight();
    static std::vector<PointLight> *getPointLights();
    static unsigned int getPointLightsSSBO();
    static int getNumberOfShownPointLights();
    static float getPointLightsRotationSpeed();
    static void clear();

private:
    // --- Private constructor
    LightManager();

    // --- Private static members
    static glm::vec3 s_ambientLight;
    static std::vector<PointLight> s_pointLights;
    static unsigned int s_pointLightsSSBO;
    static int s_numberOfShownPointLights;
    static float s_pointLightsRotationSpeed;
};


#endif // LIGHT_MANAGER_H