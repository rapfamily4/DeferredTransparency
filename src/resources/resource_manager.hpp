#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <map>
#include <string>

#include <glad/glad.h>

#include "resources/model.hpp"
#include "resources/shader.hpp"
#include "resources/texture.hpp"


// --- ResourceManager class
class ResourceManager {
	public:
		// --- Public static methods
		static Model *loadModel(std::string path);
		static Shader *loadShader(std::string name, std::string vertexPath, std::string fragmentPath, std::string geometryPath = "");
		static Texture *loadTexture(std::string path);
		static Model *getModel(std::string path);
		static Shader *getShader(std::string name);
		static Texture *getTexture(std::string path);
		static void clear();
	
	private:
		// --- Private constructor
		ResourceManager() { }
		
		// --- Private static members 
		static std::map<std::string, Model> s_models;
		static std::map<std::string, Shader> s_shaders;
		static std::map<std::string, Texture> s_textures;
};


#endif