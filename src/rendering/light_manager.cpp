#include <iostream>
#include <cstddef>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rendering/light_manager.hpp"
#include "rendering/lights.hpp"


// --- Public static members
glm::vec3 LightManager::s_ambientLight{ LIGHT_COLOR_AMBIENT };
std::vector<PointLight> LightManager::s_pointLights;
unsigned int LightManager::s_pointLightsSSBO{ 0 };
int LightManager::s_numberOfShownPointLights{ LIGHT_NUMSHOWN };
float LightManager::s_pointLightsRotationSpeed{ LIGHT_ROTSPEED };


// --- Public static functions
void LightManager::init() {
    // Create empty SSBO for point lights
    glGenBuffers(1, &s_pointLightsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_pointLightsSSBO);
    PointLight light{};
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLight) * s_pointLights.capacity(), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

PointLight *LightManager::newPointLight(glm::vec3 position, glm::vec3 color, float constant, float linear, float quadratic) {
    // Create point light istance
    PointLight light{};
    light.position.x = position.x;
    light.position.y = position.y;
    light.position.z = position.z;
    light.position.w = 1.0f; // It's useful in deferredShader.frag for matrix multiplications.
    light.color.x = color.x;
    light.color.y = color.y;
    light.color.z = color.z;
    light.color.w = 1.0f;
    light.constantLinearQuadratic.x = constant;
    light.constantLinearQuadratic.y = linear;
    light.constantLinearQuadratic.z = quadratic;

    // Keep vector's capacity before element insertion
    unsigned int prevCapacity = s_pointLights.capacity();

    // Push it to the vector
    s_pointLights.push_back(light);

    // Insert element in SSBO; allocate more memory only if needed
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_pointLightsSSBO);
    if (prevCapacity != s_pointLights.capacity())
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLight) * s_pointLights.capacity(), s_pointLights.data(), GL_DYNAMIC_DRAW);
    else
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLight) * (s_pointLights.size() - 1), sizeof(PointLight), &light);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Return element on vector
    return &s_pointLights[s_pointLights.size() - 1];
}

void LightManager::updatePointLightsSSBO(const glm::mat4 &viewMatrix, bool updateNotShown) { 
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_pointLightsSSBO);
    int size = updateNotShown ? s_pointLights.size() : s_numberOfShownPointLights;
    for (int i = 0; i < size; i++) {
        glm::vec4 vPosition = viewMatrix * s_pointLights[i].position;
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLight) * i + offsetof(PointLight, position), sizeof(glm::vec4), &vPosition);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void LightManager::setAmbientLight(glm::vec3 ambientLight) {
    s_ambientLight = ambientLight;
}

void LightManager::setNumberOfShownPointLights(int numberOfShown) {
    if (numberOfShown >= LIGHT_MAXSHOWN)
        s_numberOfShownPointLights = LIGHT_MAXSHOWN;
    else if (numberOfShown <= LIGHT_MINSHOWN)
        s_numberOfShownPointLights = LIGHT_MINSHOWN;
    else
        s_numberOfShownPointLights = numberOfShown;
}

void LightManager::setPointLightsRotationSpeed(float speed) {
    if (speed >= LIGHT_MAXROTSPEED)
        s_pointLightsRotationSpeed = LIGHT_MAXROTSPEED;
    else if (speed <= LIGHT_MINROTSPEED)
        s_pointLightsRotationSpeed = LIGHT_MINROTSPEED;
    else
        s_pointLightsRotationSpeed = speed;
}

glm::vec3 LightManager::getAmbientLight() {
    return s_ambientLight;
}

std::vector<PointLight> *LightManager::getPointLights() {
    return &s_pointLights;
}

unsigned int LightManager::getPointLightsSSBO() {
    return s_pointLightsSSBO;
}

int LightManager::getNumberOfShownPointLights() {
    return s_numberOfShownPointLights;
}

float LightManager::getPointLightsRotationSpeed() {
    return s_pointLightsRotationSpeed;
}

void LightManager::clear() {
    s_pointLights.clear();
}