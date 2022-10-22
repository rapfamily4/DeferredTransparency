// Written by Luigi Rapetta, 2022.

#ifndef INPUT_MANAGER_HPP
#define INPUT_MANAGER_HPP

#include <vector>

#include <GLFW/glfw3.h>

#include "consts.hpp"


// --- Supported input types.
enum class KeyboardType { press, hold, release, repeat };

// --- Type aliases for function pointers
using KeyboardHandler = void(*)(int, KeyboardType, float);
using MouseDeltaHandler = void(*)(float, float, float);
using MouseScrollHandler = void(*)(float, float, float);

// --- Input manager class
class InputManager {
	public:
		// --- Public static methods
		static void subscribeKeyboard(KeyboardHandler keyboardHandler);
		static void subscribeMouseDelta(MouseDeltaHandler mouseDeltaHandler);
		static void subscribeMouseScroll(MouseScrollHandler mouseScrollHandler);
		static void processKeyboard(int key, KeyboardType type, float deltaTime);
		static void processKeyboardHeld(float s_deltaTime);
		static void processMouseDelta(float deltaTime);
		static void processMouseScroll(float xoffset, float yoffset, float deltaTime);
		static void pressKey(int key);
		static void releaseKey(int key);
		static void updateMouseDelta(float xpos, float ypos);
		static bool keyIsHeld(int key);
		static void printHeldKeys();
		static void enableMouse(GLFWwindow *window, bool enable);
		static bool mouseIsEnabled();

	private:
		// --- Private constructor
		InputManager();

		// --- Private static members
		// Lists and maps of keys' status
		static bool s_firstMouse;
		static bool s_mouseIsEnabled;
		static bool s_keysHeldMap[];
		static std::vector<int> s_keysHeldVec;
		static glm::vec2 s_mouseDelta;
		static glm::vec2 s_mouseLastPosition;

		// List of functions to execute in response to keyboard events
		static std::vector<KeyboardHandler> s_keyboardHandlers;
		static std::vector<MouseDeltaHandler> s_mouseDeltaHandlers;
		static std::vector<MouseScrollHandler> s_mouseScrollHandlers;
};


#endif