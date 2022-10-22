#ifndef CONSTS_HPP
#define CONSTS_HPP

#include <cmath>
#include <string>

#include <glm/glm.hpp>


// --- Global constants
// Input
const unsigned int INPUT_KEY_NUMBER{1024};

// Camera
const glm::vec3 CAMERA_DEFAULT_POSITION{0.0f};
const glm::vec3 CAMERA_DEFAULT_FRONT{0.0f, 0.0f, -1.0f};
const glm::vec3 CAMERA_DEFAULT_WORLDUP{0.0f, 1.0f, 0.0f};
const float CAMERA_DEFAULT_YAW{ -90.0f };
const float CAMERA_DEFAULT_PITCH{ 0.0f };
const float CAMERA_DEFAULT_ZNEAR{ 0.125f };
const float CAMERA_DEFAULT_ZFAR{ 1000.0f };
const float CAMERA_DEFAULT_SPEED{ 8.0f };
const float CAMERA_DEFAULT_SENSITIVITY{ 0.3f };
const float CAMERA_DEFAULT_FOV{ 80.0f };

// Screen
const unsigned int SCREEN_DEFAULT_WIDTH{800};
const unsigned int SCREEN_DEFAULT_HEIGHT{600};

// GUI
const int GUI_DEFAULT_WIDTH{460};
const int GUI_DEFAULT_HEIGHT{320};

// Light
const glm::vec3 LIGHT_POS{ 0.0f };
const glm::vec3 LIGHT_DIR{ 0.0f };
constexpr float LIGHT_CUTOFF{ 10.0f };
constexpr float LIGHT_CUTOFF_OUTER{ 12.0f };
const glm::vec3 LIGHT_COLOR{ 1.0f };
const glm::vec3 LIGHT_COLOR_AMBIENT{ 0.25f };
constexpr float LIGHT_CONSTANT{ 1.0f };
constexpr float LIGHT_LINEAR{ 0.14f };
constexpr float LIGHT_QUADRATIC{ 0.07f };
const int LIGHT_NUMSHOWN{ 16 };
const int LIGHT_MINSHOWN{ 0 };
const int LIGHT_MAXSHOWN{ 128 };
const float LIGHT_ROTSPEED{ 15.f };
const float LIGHT_MINROTSPEED{ -180.f};
const float LIGHT_MAXROTSPEED{ 180.f };

// Material
const glm::vec4 MATERIAL_DIFFUSE{ 1.0f };
constexpr float MATERIAL_ROUGHNESS{ 0.5f };
constexpr float MATERIAL_AMBIENT_OCCLUSION{ 1.0f };
constexpr float MATERIAL_METALNESS{ 0.0f };

// Renderer
const std::string RENDERER_GBUFFER_VERTEX{ "assets/shaders/gBufferShader.vert" };
const std::string RENDERER_GBUFFER_FRAGMENT{ "assets/shaders/gBufferShader.frag" };
const std::string RENDERER_DEFERRED_VERTEX{ "assets/shaders/screenSpaceShader.vert" };
const std::string RENDERER_DEFERRED_FRAGMENT{ "assets/shaders/deferredShader.frag" };
const std::string RENDERER_SCREENSPACE_VERTEX{ "assets/shaders/screenSpaceShader.vert" };
const std::string RENDERER_SCREENSPACE_FRAGMENT{ "assets/shaders/screenSpaceShader.frag" };
const int RENDERER_DEPTHPEELING_PASSES{ 4 };
const int RENDERER_DEPTHPEELING_MINPASSES{ 1 };
const int RENDERER_DEPTHPEELING_MAXPASSES{ 16 };

// Entity
const glm::vec3 ENTITY_POS{ 0.0f };
const glm::vec3 ENTITY_ROT{ 0.0f };


#endif // CONSTS_HPP