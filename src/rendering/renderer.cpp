#include <iostream>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "consts.hpp"
#include "input/input_manager.hpp"
#include "rendering/lights.hpp"
#include "rendering/renderer.hpp"
#include "resources/resource_manager.hpp"
#include "resources/model.hpp"


// --- Private static members
bool Renderer::s_isInitialized{ false };
Camera Renderer::s_camera;
Shader *Renderer::s_gBufferShader;
Shader *Renderer::s_deferredShader;
Shader *Renderer::s_screenSpaceShader;
unsigned int Renderer::s_framebufferWidth{ 0 };
unsigned int Renderer::s_framebufferHeight{ 0 };
unsigned int Renderer::s_depthPeelingPasses{ RENDERER_DEPTHPEELING_PASSES };
unsigned int Renderer::s_opaqueFBO{ 0 };
unsigned int Renderer::s_opaqueBuffer{ 0 };
unsigned int Renderer::s_transparentGBufferFBO[2] = {0, 0};
unsigned int Renderer::s_transparentDepthBuffer[2] = {0, 0};
unsigned int Renderer::s_transparentGPosition{ 0 };
unsigned int Renderer::s_transparentGNormal{ 0 };
unsigned int Renderer::s_transparentGDiffuse{ 0 };
unsigned int Renderer::s_transparentGRoughnessMetalnessAO{ 0 };
unsigned int Renderer::s_opaqueGBufferFBO{ 0 };
unsigned int Renderer::s_opaqueDepthBuffer{ 0 };
unsigned int Renderer::s_opaqueGPosition{ 0 };
unsigned int Renderer::s_opaqueGNormal{ 0 };
unsigned int Renderer::s_opaqueGDiffuse{ 0 };
unsigned int Renderer::s_opaqueGRoughnessMetalnessAO{ 0 };
unsigned int Renderer::s_quadVAO{ 0 };
unsigned int Renderer::s_quadVBO{ 0 };


// --- Public static methods
void Renderer::init(unsigned int framebufferWidth, unsigned int framebufferHeight) {
    // Set framebuffer's resolution
    s_framebufferWidth = framebufferWidth;
    s_framebufferHeight = framebufferHeight;
    
    // Create camera
    s_camera = Camera{CAMERA_DEFAULT_POSITION, framebufferWidth, framebufferHeight};

    // Setup G-buffer
    setupFramebuffers(framebufferWidth, framebufferHeight);
    
    // Load shaders
    s_gBufferShader = ResourceManager::loadShader("gBufferShader", RENDERER_GBUFFER_VERTEX, RENDERER_GBUFFER_FRAGMENT);
    s_deferredShader = ResourceManager::loadShader("deferredShader", RENDERER_DEFERRED_VERTEX, RENDERER_DEFERRED_FRAGMENT);
    s_screenSpaceShader = ResourceManager::loadShader("screenSpaceShader", RENDERER_SCREENSPACE_VERTEX, RENDERER_SCREENSPACE_FRAGMENT);

    // Setup quad VAO and VBO
    float quadVertices[] = {
        // Positions           // Texture Coords
        -1.0f,  1.0f, 0.0f,    0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,    0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,    1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,    1.0f, 0.0f,
    };
    glGenVertexArrays(1, &s_quadVAO);
    glGenBuffers(1, &s_quadVBO);
    glBindVertexArray(s_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, s_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Subscribe to InputManager
    InputManager::subscribeKeyboard(keyboardHandler);
    InputManager::subscribeMouseDelta(mouseDeltaHandler);
    InputManager::subscribeMouseScroll(mouseScrollHandler);

    // Set renderer as initialized
    s_isInitialized = true;
}

void Renderer::renderEntities(std::vector<Entity*> *opaqueEntities, std::vector<Entity*> *transparentEntities, glm::vec3 ambientLight, unsigned int pointLightsSSBO, unsigned int pointLightsSize) {
    // Clear opaque framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, s_opaqueFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);    

    // How rendering works:
    //      1) Geometry pass for opaque entities;
    //      2) Geometry and lighting passes for transparent entities, using depth buffer computed from step 1;
    //      3) Lighting pass for opaque entities.


    // ------------------------------------------------------------------------
    // ---1--- Geometry pass for opaque entities    
    // Set blending options
    glDisable(GL_BLEND);

    // Clear G-buffer
    // By setting alpha to 0, the blending operations for the lighting pass will make the background black with alpha = 1.
    glBindFramebuffer(GL_FRAMEBUFFER, s_opaqueGBufferFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use shader on G-buffer
    s_gBufferShader->use();

    // Setup common uniforms for geometry pass
    glm::mat4 viewMatrix = s_camera.getViewMatrix();
    s_gBufferShader->setMatrix4("viewMatrix", viewMatrix);
    s_gBufferShader->setMatrix4("projectionMatrix", s_camera.getPerspectiveMatrix());
    s_gBufferShader->setInteger("executeDepthPeeling", false);
    // Run geometry pass
    for (auto iter = opaqueEntities->begin(); iter != opaqueEntities->end(); iter++)
        deferredRenderGeometry(true, (*iter), viewMatrix);
    // ------------------------------------------------------------------------
    // ---2--- Geometry and lighting passes for transparent entities
    if (transparentEntities->size() > 0) {
        // Setup common uniforms for geometry pass
        s_gBufferShader->setInteger("executeDepthPeeling", true);
        s_gBufferShader->setInteger("previousDepth", 0);
        s_gBufferShader->setInteger("opaqueDepth", 1);
        s_gBufferShader->setFloat("bufferWidth", (float)s_framebufferWidth);
        s_gBufferShader->setFloat("bufferHeight", (float)s_framebufferHeight);

        // Execute depth peeling passes
        int maxPasses = Renderer::getDepthPeelingPasses();
        for (int pass = 0; pass < maxPasses; pass++) {
            // Bind correct G-buffer FBO
            int currId = pass % 2;
            int prevId = 1 - currId;
            glBindFramebuffer(GL_FRAMEBUFFER, s_transparentGBufferFBO[currId]);

            // Clear G-buffer
            // By setting alpha to 0, the blending operations for the lighting pass will make the background black with alpha = 1.
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Disable blending for geometry pass
            glDisable(GL_BLEND);

            // Use shader on G-buffer
            s_gBufferShader->use();

            // Setup depth peeling textures for geometry pass
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, s_transparentDepthBuffer[prevId]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, s_opaqueDepthBuffer);

            // Run geometry pass
            bool first = pass == 0;
            for (auto iter = transparentEntities->begin(); iter != transparentEntities->end(); iter++)
                if ((*iter)->getMaterial()->diffuse.a >= 0.0001f)
                    deferredRenderGeometry(first, (*iter), viewMatrix);
            // Enable blending for lighting pass
            //
            // These blending settings enable front-to-back blending.
            // The front-to-back blending equation is:
            //      Cdst = Adst (Asrc Csrc) + Cdst
            //      Adst = (1-Asrc) Adst
            // The following blending settings produce:
            //      Cdst = Adst (Csrc) + Cdst
            //      Adst = (1-Asrc) Adst
            // The missing multiply by Asrc must be compensated in the shader of the lighting pass,
            // by simply multiplying the rgb component of the final color with the alpha value.
            //
            // SOURCE: https://community.khronos.org/t/front-to-back-blending/65155/3
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA); 

            // Run lighting pass
            deferredRenderLighting(true, ambientLight, pointLightsSSBO, pointLightsSize);
        }
    }

    // ------------------------------------------------------------------------
    // ---3--- Lighting pass for opaque entities
    // Enable blending for lighting pass
    //
    // These blending settings enable front-to-back blending:
    //      Cdst = Adst Csrc + Cdst
    //
    // SOURCE: https://community.khronos.org/t/front-to-back-blending/65155/3
    if (opaqueEntities->size() > 0) {
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_DST_ALPHA, GL_ONE);

        // Run lighting pass
        deferredRenderLighting(false, ambientLight, pointLightsSSBO, pointLightsSize);
    }
}

void Renderer::renderOnDefaultFramebuffer() {
    // Set blending options
    glDisable(GL_BLEND);

    // Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    s_screenSpaceShader->use();

    // Bind opaque buffer
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_opaqueBuffer);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, s_opaqueDepthBuffer);
    
    // Setup OIT-buffer textures uniforms
    s_screenSpaceShader->setInteger("opaqueBuffer", 0);
    s_screenSpaceShader->setInteger("depthBuffer", 1);

    // Draw on quad
    glBindVertexArray(s_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

bool Renderer::isInitialized() {
    return s_isInitialized;
}

void Renderer::setFramebufferResolution(unsigned int framebufferWidth, unsigned int framebufferHeight) {
    // Update framebuffer metrics
    s_framebufferWidth = framebufferWidth;
    s_framebufferHeight = framebufferHeight;
    
    // Update camera's view frustum
    s_camera.setResolution(framebufferWidth, framebufferHeight);

    // Update G-buffer textures
    setupFramebuffers(framebufferWidth, framebufferHeight);
}

void Renderer::clear() {
    s_isInitialized = false;
	glDeleteFramebuffers(1, (GLuint*)&s_opaqueFBO);
    glDeleteFramebuffers(2, (GLuint*)s_transparentGBufferFBO);
    glDeleteFramebuffers(1, (GLuint*)&s_opaqueGBufferFBO);
	glDeleteTextures(1, (GLuint*)&s_opaqueBuffer);
    glDeleteTextures(2, (GLuint*)s_transparentDepthBuffer);
	glDeleteTextures(1, (GLuint*)&s_transparentGPosition);
	glDeleteTextures(1, (GLuint*)&s_transparentGNormal);
	glDeleteTextures(1, (GLuint*)&s_transparentGDiffuse);
	glDeleteTextures(1, (GLuint*)&s_transparentGRoughnessMetalnessAO);
    glDeleteTextures(1, (GLuint*)&s_opaqueDepthBuffer);
    glDeleteTextures(1, (GLuint*)&s_opaqueGPosition);
    glDeleteTextures(1, (GLuint*)&s_opaqueGNormal);
    glDeleteTextures(1, (GLuint*)&s_opaqueGDiffuse);
    glDeleteTextures(1, (GLuint*)&s_opaqueGRoughnessMetalnessAO);
    glDeleteVertexArrays(1, (GLuint*)&s_quadVAO);
    glDeleteBuffers(1, (GLuint*)&s_quadVBO);
}

Camera& Renderer::getCamera() {
    return s_camera;
}

unsigned int Renderer::getFramebufferWidth() {
    return s_framebufferWidth;
}

unsigned int Renderer::getFramebufferHeight() {
    return s_framebufferHeight;
}

unsigned int Renderer::getDepthPeelingPasses() {
    return s_depthPeelingPasses;
}

void Renderer::setDepthPeelingPasses(int passesNumber) {
    if (passesNumber >= RENDERER_DEPTHPEELING_MAXPASSES)
        s_depthPeelingPasses = RENDERER_DEPTHPEELING_MAXPASSES;
    else if (passesNumber <= RENDERER_DEPTHPEELING_MINPASSES)
        s_depthPeelingPasses = RENDERER_DEPTHPEELING_MINPASSES;
    else
        s_depthPeelingPasses = passesNumber;
}

// --- Private static methods
void Renderer::setupFramebuffers(unsigned int framebufferWidth, unsigned int framebufferHeight) {
    // --- Opaque FBO
    // Create opaque FBO
    if (s_opaqueFBO == 0) glGenFramebuffers(1, &s_opaqueFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, s_opaqueFBO);

    // Create and attach opaque buffer
	if (s_opaqueBuffer == 0) glGenTextures(1, &s_opaqueBuffer);
	glBindTexture(GL_TEXTURE_2D, s_opaqueBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_opaqueBuffer, 0);

    // Check if opaque FBO is complete
    glBindFramebuffer(GL_FRAMEBUFFER, s_opaqueFBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER: opaque FBO not complete.\n";


    // --- Transparency G-buffer FBOs
    // Create position buffer for tranparency rendering
    if (s_transparentGPosition == 0) glGenTextures(1, &s_transparentGPosition);
    glBindTexture(GL_TEXTURE_2D, s_transparentGPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create normal buffer for tranparency rendering
    if (s_transparentGNormal == 0) glGenTextures(1, &s_transparentGNormal);
    glBindTexture(GL_TEXTURE_2D, s_transparentGNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create diffuse buffer for tranparency rendering
    if (s_transparentGDiffuse == 0) glGenTextures(1, &s_transparentGDiffuse);
    glBindTexture(GL_TEXTURE_2D, s_transparentGDiffuse);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create roughness + metalness + ambient occlusion buffer for tranparency rendering
    if (s_transparentGRoughnessMetalnessAO == 0) glGenTextures(1, &s_transparentGRoughnessMetalnessAO);
    glBindTexture(GL_TEXTURE_2D, s_transparentGRoughnessMetalnessAO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create G-buffer FBOs for tranparency rendering
    for (int i = 0; i < 2; i++) {
        // Create FBO
        if (s_transparentGBufferFBO[i] == 0) glGenFramebuffers(1, &s_transparentGBufferFBO[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, s_transparentGBufferFBO[i]);

        // Attach color buffers to this G-buffer
        glBindTexture(GL_TEXTURE_2D, s_transparentGPosition);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_transparentGPosition, 0);
        glBindTexture(GL_TEXTURE_2D, s_transparentGNormal);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, s_transparentGNormal, 0);
        glBindTexture(GL_TEXTURE_2D, s_transparentGDiffuse);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, s_transparentGDiffuse, 0);
        glBindTexture(GL_TEXTURE_2D, s_transparentGRoughnessMetalnessAO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, s_transparentGRoughnessMetalnessAO, 0);

        // Tell OpenGL which color attachments the G-buffer FBOs will use for rendering
        unsigned int gAttachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
        glDrawBuffers(4, gAttachments);

        // Create and attach depth buffer
        if (s_transparentDepthBuffer[i] == 0) glGenTextures(1, &s_transparentDepthBuffer[i]);
        glBindTexture(GL_TEXTURE_2D, s_transparentDepthBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, framebufferWidth, framebufferHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, s_transparentDepthBuffer[i], 0);
        // If you want to let shaders use these buffers with sampler2DShadow, set the following parameter for both textures:
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    
        // Check if the FBO is complete
        glBindFramebuffer(GL_FRAMEBUFFER, s_transparentGBufferFBO[i]);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER: Transparency G-buffer(" << i << ") FBO not complete.\n";
    }


    // --- Opaque G-buffer FBO
    // Create position buffer for opaque rendering
    if (s_opaqueGPosition == 0) glGenTextures(1, &s_opaqueGPosition);
    glBindTexture(GL_TEXTURE_2D, s_opaqueGPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create normal buffer for opaque rendering
    if (s_opaqueGNormal == 0) glGenTextures(1, &s_opaqueGNormal);
    glBindTexture(GL_TEXTURE_2D, s_opaqueGNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create diffuse buffer for opaque rendering
    if (s_opaqueGDiffuse == 0) glGenTextures(1, &s_opaqueGDiffuse);
    glBindTexture(GL_TEXTURE_2D, s_opaqueGDiffuse);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create roughness + metalness + ambient occlusion buffer for opaque rendering
    if (s_opaqueGRoughnessMetalnessAO == 0) glGenTextures(1, &s_opaqueGRoughnessMetalnessAO);
    glBindTexture(GL_TEXTURE_2D, s_opaqueGRoughnessMetalnessAO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create G-buffer FBOs for tranparency rendering
    if (s_opaqueGBufferFBO == 0) glGenFramebuffers(1, &s_opaqueGBufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, s_opaqueGBufferFBO);

    // Attach color buffers to this G-buffer
    glBindTexture(GL_TEXTURE_2D, s_opaqueGPosition);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_opaqueGPosition, 0);
    glBindTexture(GL_TEXTURE_2D, s_opaqueGNormal);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, s_opaqueGNormal, 0);
    glBindTexture(GL_TEXTURE_2D, s_opaqueGDiffuse);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, s_opaqueGDiffuse, 0);
    glBindTexture(GL_TEXTURE_2D, s_opaqueGRoughnessMetalnessAO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, s_opaqueGRoughnessMetalnessAO, 0);

    // Tell OpenGL which color attachments the G-buffer FBOs will use for rendering
    unsigned int gAttachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, gAttachments);

    // Create and attach depth buffer
    if (s_opaqueDepthBuffer == 0) glGenTextures(1, &s_opaqueDepthBuffer);
    glBindTexture(GL_TEXTURE_2D, s_opaqueDepthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, framebufferWidth, framebufferHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, s_opaqueDepthBuffer, 0);
    // If you want to let shaders use these buffers with sampler2DShadow, set the following parameter for both textures:
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

    // Check if the FBO is complete
    glBindFramebuffer(GL_FRAMEBUFFER, s_opaqueGBufferFBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER: Opaque G-buffer FBO not complete.\n";


    // --- Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);


    // NOTE FOR RESIZE: glTexImage2D allows to resize the buffer (https://stackoverflow.com/questions/23362497/how-can-i-resize-existing-texture-attachments-at-my-framebuffer)
    //                  glRenderbufferStorage doesn't: you need to recreate the whole thing
    //
    //                  So, you don't have to destroy the buffer first and generate a new one.
    //                  Remember the GLuint is only a 'pointer' of sorts for OpenGL. It is not
    //                  the actual storage, thus it is perfectly fine to reuse the same 'pointer'
    //                  (if you did not delete it yet of course).
}

void Renderer::deferredRenderGeometry(bool firstPass, Entity *entity, glm::mat4 &viewMatrix) {
    // Setup matrices for this entity
    glm::mat4 modelMatrix = glm::mat4{ 1.0f };
    glm::mat3 normalMatrix = glm::mat3{ 1.0f };
    modelMatrix = glm::translate(modelMatrix, entity->getPosition());
    normalMatrix = glm::inverseTranspose(glm::mat3(viewMatrix * modelMatrix));
    s_gBufferShader->setMatrix4("modelMatrix", modelMatrix);
    s_gBufferShader->setMatrix3("normalMatrix", normalMatrix);

    // Setup material uniforms
    Material *material = entity->getMaterial();
    s_gBufferShader->setVector4("material.diffuse", material->diffuse);
    s_gBufferShader->setFloat("material.roughness", material->roughness);
    s_gBufferShader->setFloat("material.metalness", material->metalness);
    s_gBufferShader->setFloat("material.ambientOcclusion", material->ambientOcclusion);

    // State whether this is the first depth peeling pass
    s_gBufferShader->setInteger("firstPass", firstPass);

    // Build VAO list
    Model *model = entity->getModel();
    int meshNum = model->getMeshesNumber();
    GLuint *vaoList = new GLuint[meshNum];
    int *indicesNumberList = new int[meshNum];
    model->buildMeshLists(vaoList, indicesNumberList);

    // Render
    for (int i = 0; i < meshNum; i++) {
        glBindVertexArray(vaoList[i]);
        glDrawElements(GL_TRIANGLES, indicesNumberList[i], GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);

    // Deallocate lists
    delete[] vaoList;
    delete[] indicesNumberList;
}

void Renderer::deferredRenderLighting(bool transparentGBuffer, glm::vec3 ambientLight, unsigned int pointLightsSSBO, unsigned int pointLightsSize) {  
    // Use shader on opaque framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, s_opaqueFBO);
    s_deferredShader->use();

    // Bind G-buffer textures
    if (transparentGBuffer) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, s_transparentGPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, s_transparentGNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, s_transparentGDiffuse);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, s_transparentGRoughnessMetalnessAO);
    } else {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, s_opaqueGPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, s_opaqueGNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, s_opaqueGDiffuse);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, s_opaqueGRoughnessMetalnessAO);
    }
    
    // Setup G-buffer textures uniforms
    s_deferredShader->setInteger("gPosition", 0);
    s_deferredShader->setInteger("gNormal", 1);
    s_deferredShader->setInteger("gDiffuse", 2);
    s_deferredShader->setInteger("gRoughnessMetalnessAO", 3);

    // Setup lights
    s_deferredShader->setVector3("ambientLight", ambientLight);
    s_deferredShader->setInteger("pointLightsNumber", pointLightsSize);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pointLightsSSBO);

    // Setup subroutines
    s_deferredShader->setSubroutineUniform(GL_FRAGMENT_SHADER, "LocalModel", "GGX");
    glBindVertexArray(s_quadVAO);
    s_deferredShader->loadSubroutines(GL_FRAGMENT_SHADER);

    // Draw on quad
    glBindVertexArray(s_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::keyboardHandler(int key, KeyboardType type, float deltaTime) {
    // Move the camera
    if (!InputManager::mouseIsEnabled())
        s_camera.keyboardHandler(key, type, deltaTime);
}

void Renderer::mouseDeltaHandler(float xdelta, float ydelta, float deltaTime) {
    // Rotate the camera
    if (!InputManager::mouseIsEnabled())
        s_camera.mouseDeltaHandler(xdelta, ydelta, deltaTime);
}

void Renderer::mouseScrollHandler(float xdelta, float ydelta, float deltaTime) {
    // Zoom the camera
    if (!InputManager::mouseIsEnabled())
        s_camera.mouseScrollHandler(xdelta, ydelta, deltaTime);
}