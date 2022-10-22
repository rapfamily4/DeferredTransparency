#include <iostream>
#include <sstream>
#include <fstream>
#include <utility>

#include <stb_image.h>

#include "resources/resource_manager.hpp"


// --- Public static members
std::map<std::string, Model> ResourceManager::s_models;
std::map<std::string, Shader> ResourceManager::s_shaders;
std::map<std::string, Texture> ResourceManager::s_textures;


// --- Public static methods
Model *ResourceManager::loadModel(std::string path) {
    // Loading though Assimp
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

    // Check for errors
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << "\n";
        return nullptr;
    }

	// Store and return
	s_models[path] = Model{};
	s_models[path].setup(scene);
	return &s_models[path];
}

Shader *ResourceManager::loadShader(std::string name, std::string vertexPath, std::string fragmentPath, std::string geometryPath) {
	// Convert strings into arrays of characters
	const char *cVertexPath = vertexPath.c_str();
	const char *cFragmentPath = fragmentPath.c_str();
	const char *cGeometryPath = geometryPath == "" ? nullptr : geometryPath.c_str();

	// Read shader files
	std::string vertexCode;
    std::string fragmentCode;
	std::string geometryCode;
	try {
		// Open files
		std::ifstream vertexFile(cVertexPath);
		std::ifstream fragmentFile(cFragmentPath);
		std::stringstream vertexStream, fragmentStream;
		
		// Put buffers into streams
		vertexStream << vertexFile.rdbuf();
		fragmentStream << fragmentFile.rdbuf();
		
		// Close file handlers
		vertexFile.close();
		fragmentFile.close();
		
		// Convert stream into string
		vertexCode = vertexStream.str();
		fragmentCode = fragmentStream.str();
		
		// Handle geometry shaders
		if (cGeometryPath != nullptr) {
			std::ifstream geometryFile(cGeometryPath);
			std::stringstream geometryStream;
			geometryStream << geometryFile.rdbuf();
			geometryFile.close();
			geometryCode = geometryStream.str();
		}
	} catch (std::exception e) {
		std::cout << "ERROR::SHADER: failed to read shader files\n"; // better error messages whould be appreciated (state which file caused issues)
	}
	
	// Compile shader program from source files
	const char *cVertexCode = vertexCode.c_str();
	const char *cFragmentCode = fragmentCode.c_str();
	const char *cGeometryCode = geometryCode.c_str();
	Shader shader;
	shader.compile(cVertexCode, cFragmentCode, cGeometryPath != nullptr ? cGeometryCode : nullptr);

	// Setup subroutines
	shader.setupSubroutines(GL_VERTEX_SHADER);
	shader.setupSubroutines(GL_FRAGMENT_SHADER);
	if (cGeometryPath != nullptr) shader.setupSubroutines(GL_GEOMETRY_SHADER);

	// Store and return
	s_shaders[name] = shader;
	return &s_shaders[name];
}

Texture *ResourceManager::loadTexture(std::string path) {
	// Convert string into array of characters
	const char *cPath = path.c_str();

	// Load image
	int width, height, nrChannels;
	unsigned char *data = stbi_load(cPath, &width, &height, &nrChannels, 0);
	
	// Warn loading error
	if (data == NULL)
		printf("ERROR::TEXTURE: failed to load \"%s\"\n", cPath);
		
	// Create texture object
	Texture texture{nrChannels > 3};
	
	// Generate texture
	texture.generate(width, height, data);
	
	// Deallocate image data
	stbi_image_free(data);

	// Store and return
	s_textures[path] = texture;
	return &s_textures[path];
}

Model *ResourceManager::getModel(std::string path) {
	return &s_models[path];
}

Shader *ResourceManager::getShader(std::string name) {
	return &s_shaders[name];
}

Texture *ResourceManager::getTexture(std::string path) {
	return &s_textures[path];
}

void ResourceManager::clear() {
	// Clear shaders
	for (std::pair<const std::string, Shader> iter : s_shaders)
		glDeleteProgram(iter.second.getID());
	
	// Clear textures
	for (std::pair<const std::string, Texture> iter : s_textures) {
		unsigned int id{iter.second.getID()};
		glDeleteTextures(1, &id);
	}
}