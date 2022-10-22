#include <iostream>
#include <map>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "consts.hpp"
#include "debug.hpp"
#include "context_manager.hpp"
#include "input/input_manager.hpp"
#include "rendering/light_manager.hpp"
#include "rendering/renderer.hpp"


/* STATIC MEMBERS */
GLFWwindow *ContextManager::s_window;
ImGuiIO *ContextManager::s_imGuiIO;
float ContextManager::s_deltaTime;
float ContextManager::s_lastFrame;
unsigned int ContextManager::s_windowWidth;
unsigned int ContextManager::s_windowHeight;


// --- Public static members
void ContextManager::init(std::string title, unsigned int width, unsigned int height) {
	// Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // Uncomment if you want a DEBUG context
	//glfwWindowHint(GLFW_RESIZABLE, false); // Uncomment if you want the window to retain its metrics
	
	// Create window and make context current on the execution thread
	s_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(s_window);	

	// GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "ERROR::GLAD: initialization failed";
		// Throw exception
	}
	
	// Fetch viewport and framebuffer metrics
	s_windowWidth = width;
	s_windowHeight = height;
	unsigned int framebufferWidth;
	unsigned int framebufferHeight;
	glfwGetFramebufferSize(s_window, (int*)&framebufferWidth, (int*)&framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

	// Setup render state  
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	
	// Set sticky keys
	// From GLFW's documentation: When sticky keys mode is enabled, the pollable state of a key
	//							  will remain GLFW_PRESS until the state of that key is polled
	//							  with glfwGetKey. Once it has been polled, if a key release event
	//							  had been processed in the meantime, the state will reset to
	//							  GLFW_RELEASE, otherwise it will remain GLFW_PRESS.
	glfwSetInputMode(s_window, GLFW_STICKY_KEYS, GLFW_TRUE);

	// Set callbacks
	setCallbacks();

	// Setup debug mode, if requested
	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		std::cout << "*** DEBUG MODE ***\n";
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
	}
	
	// Initialize deltatime
	s_lastFrame = 0.0f;
	updateDeltaTime();

	// Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    s_imGuiIO = &ImGui::GetIO();
	//s_imGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //s_imGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends for Dear ImGui
	const char* glsl_version = "#version 460";
    ImGui_ImplGlfw_InitForOpenGL(s_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

	// Setup default position and size of Dear ImGui window.
	const ImGuiViewport *mainViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 16, mainViewport->WorkPos.y + 16));
    ImGui::SetNextWindowSize(ImVec2(GUI_DEFAULT_WIDTH, GUI_DEFAULT_HEIGHT), ImGuiCond_FirstUseEver);

	// Subscribe ContextManager::keyboardHandler to InputManager and disable mouse
	InputManager::enableMouse(s_window, true);
	InputManager::subscribeKeyboard(keyboardHandler);
	
	// Initialize renderer and other managers
	Renderer::init(framebufferWidth, framebufferHeight);
	LightManager::init();
}

void ContextManager::next() {
    // Swap double buffers
    glfwSwapBuffers(s_window);
	
	// Check and call events
	glfwPollEvents();

	// Process input for held keys
	InputManager::processKeyboardHeld(s_deltaTime);

	// Update deltatime
	updateDeltaTime();
}

void ContextManager::displayGUI() {
	// Display only if mouse is enabled
	if (!InputManager::mouseIsEnabled()) return;

	// Feed inputs to Dear ImGui; start new frame
	glDisable(GL_BLEND);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Define GUI
	ImGui::Begin("Editing tool");
	if (ImGui::CollapsingHeader("Help and Credits")) {
		ImGui::Text("ABOUT THIS DEMO:");
		ImGui::BulletText("This OpenGL 4.6 renderer implements deferred shading.");
		ImGui::BulletText("Transparency is handled with depth-peeling.");
		ImGui::BulletText("Each peel is deferred as well.");
		ImGui::Separator();
		ImGui::Text("ABOUT EDITING TOOL:");
		ImGui::BulletText("You can edit some aspects of the rendering process and the scene.");
		ImGui::BulletText("Changing the alpha of entities dynamically is fully supported.");
		ImGui::BulletText("Feel free to experiment.");
		ImGui::Separator();
		ImGui::Text("CONTROLS:");
		ImGui::BulletText("ALT: toggle edit/view mode.");
		ImGui::BulletText("W: move the camera forward in view mode.");
		ImGui::BulletText("A: move the camera to the left in view mode.");
		ImGui::BulletText("S: move the camera forward in view mode.");
		ImGui::BulletText("D: move the camera to the right in view mode.");
		ImGui::BulletText("Q: move the camera downward in view mode.");
		ImGui::BulletText("E: move the camera upward in view mode.");
		ImGui::BulletText("MOUSE WHEEL UP: zoom-in the camera in view mode.");
		ImGui::BulletText("MOUSE WHEEL DOWN: zoom-out the camera in view mode.");
		ImGui::BulletText("ESCAPE: quit the application.");
		ImGui::Separator();
		ImGui::Text("CREDITS:");
		ImGui::BulletText("Programmed by Luigi Rapetta (2022).");
		ImGui::BulletText("Mesh and Model implementations by Davide Gadia (Università degli Studi di Milano) and Michael Marchesan.");
		ImGui::BulletText("Original teapot model by Martin Newell (University of Utah).");
		ImGui::Text("");
	}
	if (ImGui::CollapsingHeader("Depth Peeling")) {
		int passes = Renderer::getDepthPeelingPasses();
		if (ImGui::SliderInt("Passes", &passes, RENDERER_DEPTHPEELING_MINPASSES, RENDERER_DEPTHPEELING_MAXPASSES))
			Renderer::setDepthPeelingPasses(passes);
	}
	if (ImGui::CollapsingHeader("Lighting")) {
		if (ImGui::TreeNode("Ambient Light")) {
			glm::vec3 ambientColor = LightManager::getAmbientLight();
			ImVec4 imguiAmbientColor{ambientColor.r, ambientColor.g, ambientColor.b, 1.f};
			if (ImGui::ColorEdit3("Color", (float*)&imguiAmbientColor, ImGuiColorEditFlags_NoOptions)) {
				ambientColor.r = imguiAmbientColor.x;
				ambientColor.g = imguiAmbientColor.y;
				ambientColor.b = imguiAmbientColor.z;
				LightManager::setAmbientLight(ambientColor);
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Point Lights")) {
			int numberOfShownPointLights = LightManager::getNumberOfShownPointLights();
			if (ImGui::SliderInt("Number shown", &numberOfShownPointLights, LIGHT_MINSHOWN, LIGHT_MAXSHOWN))
				LightManager::setNumberOfShownPointLights(numberOfShownPointLights);
			float pointLightsRotationSpeed = LightManager::getPointLightsRotationSpeed();
			if (ImGui::SliderFloat("Rotation speed", &pointLightsRotationSpeed, LIGHT_MINROTSPEED, LIGHT_MAXROTSPEED))
				LightManager::setPointLightsRotationSpeed(pointLightsRotationSpeed);
			ImGui::TreePop();
		}
	}
	if (ImGui::CollapsingHeader("Materials")) {
		std::map<std::string, Material> *materialsMap = MaterialManager::getMaterials();
		for (auto iter = materialsMap->begin(); iter != materialsMap->end(); ++iter) {
			if (ImGui::TreeNode(iter->first.c_str())) {
				bool edited = false;
				glm::vec4 diffuse = iter->second.diffuse;
				ImVec4 imguiDiffuse{diffuse.r, diffuse.g, diffuse.b, diffuse.a};
				if (ImGui::ColorEdit4("Diffuse", (float*)&imguiDiffuse, ImGuiColorEditFlags_NoOptions)) {
					diffuse.r = imguiDiffuse.x;
					diffuse.g = imguiDiffuse.y;
					diffuse.b = imguiDiffuse.z;
					diffuse.a = imguiDiffuse.w;
					iter->second.diffuse = diffuse;
					MaterialManager::updateAssignedEntities(&(iter->second));
				}
				ImGui::SliderFloat("Roughness", &(iter->second.roughness), 0.f, 1.f);
				ImGui::SliderFloat("Metalness", &(iter->second.metalness), 0.f, 1.f);
				ImGui::SliderFloat("Ambient Occlusion", &(iter->second.ambientOcclusion), 0.f, 1.f);
				ImGui::TreePop();
			}
		}
	}
	ImGui::End();

	// Render GUI
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool ContextManager::shouldClose() {
	return glfwWindowShouldClose(s_window);
}

void ContextManager::clear() {
	// Clear Dear ImGui out
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Terminate GLFW and exit
    glfwDestroyWindow(s_window);
	glfwTerminate();
}

float ContextManager::getDeltaTime() {
	return s_deltaTime;
}

unsigned int ContextManager::getWindowWidth() {
	return s_windowWidth;
}

unsigned int ContextManager::getWindowHeight() {
	return s_windowHeight;
}


// --- Private static members
void ContextManager::setCallbacks() {
	glfwSetFramebufferSizeCallback(s_window, [](GLFWwindow* window, int width, int height) {
		// Set viewport with framebuffer metrics
		glViewport(0, 0, width, height); // width and height refer to the framebuffer
		
		// Update framebuffer's metrics
		if (Renderer::isInitialized()) Renderer::setFramebufferResolution(width, height);

		// Store window metrics
		glfwGetWindowSize(s_window, (int*)&ContextManager::s_windowWidth, (int*)&ContextManager::s_windowHeight);
	});
	
	glfwSetCursorPosCallback(s_window, [](GLFWwindow* window, double xpos, double ypos) {
		InputManager::updateMouseDelta((float)xpos, (float)ypos);
		InputManager::processMouseDelta(s_deltaTime);
	});

	glfwSetScrollCallback(s_window, [](GLFWwindow* window, double xoffset, double yoffset) {
		InputManager::processMouseScroll((float)xoffset, (float)yoffset, s_deltaTime);
	});

	glfwSetKeyCallback(s_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		switch (action) {
			default:
				break;
			case GLFW_PRESS:
				if (!InputManager::keyIsHeld(key)) InputManager::pressKey(key);
				InputManager::processKeyboard(key, KeyboardType::press, s_deltaTime);
				break;
			case GLFW_REPEAT:
				// Assume that a key cannot be repeated if it wasn't pressed before
				InputManager::processKeyboard(key, KeyboardType::repeat, s_deltaTime);
				break;
			case GLFW_RELEASE:
				InputManager::releaseKey(key);
				InputManager::processKeyboard(key, KeyboardType::release, s_deltaTime);
				break;
		}
	});
}

void ContextManager::keyboardHandler(int key, KeyboardType type, float deltaTime) {
	switch (key) {
        default:
			break;
		case GLFW_KEY_ESCAPE:
            if(type == KeyboardType::press)
                glfwSetWindowShouldClose(s_window, GLFW_TRUE);
            break;
		case GLFW_KEY_LEFT_ALT:
            if(type == KeyboardType::press)
                InputManager::enableMouse(s_window, !InputManager::mouseIsEnabled());
            break;
    }
}

void ContextManager::updateDeltaTime() {
	float currentFrame{(float)glfwGetTime()};
	s_deltaTime = currentFrame - s_lastFrame;
	s_lastFrame = currentFrame;
}