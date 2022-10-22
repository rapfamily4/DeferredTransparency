// Written by Luigi Rapetta, 2022.

#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "input/input_manager.hpp"


// --- Static members
bool InputManager::s_firstMouse = true;
bool InputManager::s_mouseIsEnabled = false;
bool InputManager::s_keysHeldMap[INPUT_KEY_NUMBER];
std::vector<int> InputManager::s_keysHeldVec{};
glm::vec2 InputManager::s_mouseDelta{ 0.0f };
glm::vec2 InputManager::s_mouseLastPosition{ 0.0f };
std::vector<KeyboardHandler> InputManager::s_keyboardHandlers{};
std::vector<MouseDeltaHandler> InputManager::s_mouseDeltaHandlers{};
std::vector<MouseScrollHandler> InputManager::s_mouseScrollHandlers{};


// --- Public static methods
void InputManager::subscribeKeyboard(KeyboardHandler keyboardHandler) {
	s_keyboardHandlers.push_back(keyboardHandler);
}

void InputManager::subscribeMouseDelta(MouseDeltaHandler mouseDeltaHandler) {
	s_mouseDeltaHandlers.push_back(mouseDeltaHandler);
}

void InputManager::subscribeMouseScroll(MouseScrollHandler mouseScrollHandler) {
	s_mouseScrollHandlers.push_back(mouseScrollHandler);
}

void InputManager::processKeyboard(int key, KeyboardType type, float deltaTime) {
	for (KeyboardHandler iter : s_keyboardHandlers)
		iter(key, type, deltaTime);
}

void InputManager::processKeyboardHeld(float s_deltaTime) {
	for (int iter : s_keysHeldVec)
		processKeyboard(iter, KeyboardType::hold, s_deltaTime);
}

void InputManager::processMouseDelta(float deltaTime) {
	for (MouseDeltaHandler iter : s_mouseDeltaHandlers)
		iter(s_mouseDelta.x, s_mouseDelta.y, deltaTime);
}

void InputManager::processMouseScroll(float xoffset, float yoffset, float deltaTime) {
	for (MouseScrollHandler iter : s_mouseScrollHandlers)
		iter(xoffset, yoffset, deltaTime);
}

void InputManager::pressKey(int key) {
	if (!s_keysHeldMap[key]) s_keysHeldVec.push_back(key);
	s_keysHeldMap[key] = true;
}

void InputManager::releaseKey(int key) {
	s_keysHeldMap[key] = false;
	for (std::vector<int>::iterator iter = s_keysHeldVec.begin(); iter != s_keysHeldVec.end(); iter++)
		if (*iter == key) {
			s_keysHeldVec.erase(iter);
			break;
		}
		
}

void InputManager::updateMouseDelta(float xpos, float ypos) {
	// Set previous state as current one if there's not any
	if (s_firstMouse) {
		s_mouseLastPosition.x = xpos;
		s_mouseLastPosition.y = ypos;
		s_firstMouse = false;
	}

	// Update mouse delta
	s_mouseDelta.x = xpos - s_mouseLastPosition.x;
	s_mouseDelta.y = s_mouseLastPosition.y - ypos;

	// Set previous state
	s_mouseLastPosition.x = xpos;
	s_mouseLastPosition.y = ypos;
}

bool InputManager::keyIsHeld(int key) {
	return s_keysHeldMap[key];
}

void InputManager::printHeldKeys() {
	bool first{true};
	std::cout << "HELD:  ";
	for (int iter : s_keysHeldVec) {
		if (!first) std::cout << ", ";
		first = false;
		std::cout << iter;
	}
	std::cout << "\n";
}

void InputManager::enableMouse(GLFWwindow *window, bool enable) {
	glfwSetInputMode(window, GLFW_CURSOR, enable ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	s_mouseIsEnabled = enable;
}

bool InputManager::mouseIsEnabled() {
	return s_mouseIsEnabled;
}