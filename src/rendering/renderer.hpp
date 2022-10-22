#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <list>
#include <vector>

#include <glm/glm.hpp>

#include "consts.hpp"
#include "input/input_manager.hpp"
#include "rendering/lights.hpp"
#include "rendering/material_manager.hpp"
#include "rendering/camera.hpp"
#include "resources/shader.hpp"
#include "resources/model.hpp"
#include "scene/entity.hpp"


// --- Render class
class Renderer {
	public:		
		// --- Public static methods
		static void init(unsigned int framebufferWidth, unsigned int framebufferHeight);
		static void renderEntities(std::vector<Entity*> *opaqueEntities, std::vector<Entity*> *transparentEntities, glm::vec3 ambientLight, unsigned int pointLightsSSBO, unsigned int pointLightsSize);
		static void renderOnDefaultFramebuffer();
		static bool isInitialized();
		static void setFramebufferResolution(unsigned int framebufferWidth, unsigned int framebufferHeight);
		static void clear();
		static Camera& getCamera();
		static unsigned int getFramebufferWidth();
		static unsigned int getFramebufferHeight();
		static unsigned int getDepthPeelingPasses();
		static void setDepthPeelingPasses(int passesNumber);
		
	private:
		// --- Private constructor
		Renderer();
		
		// --- Private static methods
		static void setupFramebuffers(unsigned int framebufferWidth, unsigned int framebufferHeight);
		static void deferredRenderGeometry(bool firstPass, Entity *entity, glm::mat4 &viewMatrix);
		static void deferredRenderLighting(bool transparentGBuffer, glm::vec3 ambientLight, unsigned int pointLightsSSBO, unsigned int pointLightsSize);
		static void keyboardHandler(int key, KeyboardType type, float deltaTime);
		static void mouseDeltaHandler(float xdelta, float ydelta, float deltaTime);
		static void mouseScrollHandler(float xdelta, float ydelta, float deltaTime);

		// --- Private static members
		static bool s_isInitialized;
        static Camera s_camera;
		static Shader *s_gBufferShader;
		static Shader *s_deferredShader;
		static Shader *s_screenSpaceShader;
		static unsigned int s_framebufferWidth;
		static unsigned int s_framebufferHeight;
		static unsigned int s_depthPeelingPasses;
		static unsigned int s_opaqueFBO;
		static unsigned int s_opaqueBuffer;
		static unsigned int s_transparentGBufferFBO[2];
		static unsigned int s_transparentDepthBuffer[2];
		static unsigned int s_transparentGPosition;
		static unsigned int s_transparentGNormal;
		static unsigned int s_transparentGDiffuse;
		static unsigned int s_transparentGRoughnessMetalnessAO;
		static unsigned int s_opaqueGBufferFBO;
		static unsigned int s_opaqueDepthBuffer;
		static unsigned int s_opaqueGPosition;
		static unsigned int s_opaqueGNormal;
		static unsigned int s_opaqueGDiffuse;
		static unsigned int s_opaqueGRoughnessMetalnessAO;
		static unsigned int s_quadVAO;
		static unsigned int s_quadVBO;	
};


#endif // RENDERER_HPP