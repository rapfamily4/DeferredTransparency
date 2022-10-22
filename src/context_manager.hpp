#ifndef CONTEXT_MANAGER_HPP
#define CONTEXT_MANAGER_HPP

#include <string>

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "consts.hpp"
#include "input/input_manager.hpp"


// --- Context manager class
class ContextManager {
	public:
		// --- Public static methods
		// Initializes the manager, given the window metrics
		static void init(std::string title = "", unsigned int width = SCREEN_DEFAULT_WIDTH, unsigned int height = SCREEN_DEFAULT_HEIGHT);
		
		// Swaps double buffers, updates deltatime and manages event calls
		static void next();

		// Wrapper function to ImGui features in order to display the GUI
		static void displayGUI();
		
		// States if the window should close
		static bool shouldClose();
		
		// Clears window and framebuffer data
		static void clear();

		// Getters
		static float getDeltaTime();
		static unsigned int getWindowWidth();
		static unsigned int getWindowHeight();
		
	private:
		// --- Private constructor
		ContextManager();
		
		// --- Private static methods
		static void setCallbacks();
		static void keyboardHandler(int key, KeyboardType type, float deltaTime);
		static void updateDeltaTime();
		
		// --- Private static members
		static GLFWwindow *s_window;
		static ImGuiIO *s_imGuiIO;
		static float s_deltaTime;
		static float s_lastFrame;
		static unsigned int s_windowWidth;
		static unsigned int s_windowHeight;
};


#endif // CONTEXT_MANAGER_HPP