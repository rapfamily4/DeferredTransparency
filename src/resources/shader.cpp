#include <iostream>
#include <stdlib.h>
#include <string>
#include <utility>

#include <glad/glad.h>

#include "resources/shader.hpp"


// --- Constructor and destructor
Shader::Shader() :
	m_id{0},
	m_subroutineConfigurationVertex{ nullptr },
	m_subroutineConfigurationFragment{ nullptr },
	m_subroutineConfigurationGeometry{ nullptr },
	m_subroutineUniformsVertex{},
	m_subroutineUniformsFragment{},
	m_subroutineUniformsGeometry{},
	m_subroutinesVertex{},
	m_subroutinesFragment{},
	m_subroutinesGeometry{} {
	// ---
}

Shader::~Shader() {
	delete(m_subroutineConfigurationVertex);
	delete(m_subroutineConfigurationFragment);
	delete(m_subroutineConfigurationGeometry);
}


// --- Public methods
Shader *Shader::use() {
	glUseProgram(m_id);
	return this;
}

void Shader::compile(const char* vertexSource, const char* fragmentSource, const char* geometrySource) {
	unsigned int sVertex, sFragment, sGeometry;
	
	// Vertex shader
	sVertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(sVertex, 1, &vertexSource, NULL);
	glCompileShader(sVertex);
	checkCompileErrors(sVertex, SHADER_ERROR_VERTEX);
	
	// Fragment shader
	sFragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(sFragment, 1, &fragmentSource, NULL);
	glCompileShader(sFragment);
	checkCompileErrors(sFragment, SHADER_ERROR_FRAGMENT);
	
	// Geometry shader (optional)
	if (geometrySource != nullptr) {
		sGeometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(sGeometry, 1, &geometrySource, NULL);
		glCompileShader(sGeometry);
		checkCompileErrors(sGeometry, SHADER_ERROR_GEOMETRY);
	}
	
	// Shader program
	this->m_id = glCreateProgram();
	glAttachShader(m_id, sVertex);
	glAttachShader(m_id, sFragment);
	if (geometrySource != nullptr)
		glAttachShader(m_id, sGeometry);
	glLinkProgram(m_id);
	checkCompileErrors(m_id, SHADER_ERROR_PROGRAM);
	
	// Delete shader objects
	glDeleteShader(sVertex);
	glDeleteShader(sFragment);
	if(geometrySource != nullptr)
		glDeleteShader(sGeometry);
}

void Shader::setFloat(const char *name, float value) {
	glUniform1f(glGetUniformLocation(m_id, name), value);
}

void Shader::setInteger(const char *name, int value) {
	glUniform1i(glGetUniformLocation(m_id, name), value);
}

void Shader::setVector2(const char *name, float x, float y) {
	glUniform2f(glGetUniformLocation(m_id, name), x, y);
}

void Shader::setVector2(const char *name, const glm::vec2 &value) {
	glUniform2f(glGetUniformLocation(m_id, name), value.x, value.y);
}

void Shader::setVector3(const char *name, float x, float y, float z) {
	glUniform3f(glGetUniformLocation(m_id, name), x, y, z);
}

void Shader::setVector3(const char *name, const glm::vec3 &value) {
	glUniform3f(glGetUniformLocation(m_id, name), value.x, value.y, value.z);
}

void Shader::setVector4(const char *name, float x, float y, float z, float w) {
	glUniform4f(glGetUniformLocation(m_id, name), x, y, z, w);
}

void Shader::setVector4(const char *name, const glm::vec4 &value) {
	glUniform4f(glGetUniformLocation(m_id, name), value.x, value.y, value.z, value.w);
}

void Shader::setMatrix3(const char *name, const glm::mat3 &matrix) {
	glUniformMatrix3fv(glGetUniformLocation(m_id, name), 1, false, glm::value_ptr(matrix));
}

void Shader::setMatrix4(const char *name, const glm::mat4 &matrix) {
	glUniformMatrix4fv(glGetUniformLocation(m_id, name), 1, false, glm::value_ptr(matrix));
}

void Shader::setupSubroutines(GLenum shadertype) {
	// SOURCES: https://stackoverflow.com/questions/18498767/multiple-subroutine-types-defined-in-the-same-fragment-shader-does-not-work-corr
	//			https://www.khronos.org/opengl/wiki/Program_Introspection#Interface_query
	// Initialize
	int len;
	bool canAllocate = false;
	GLenum programInterface;
	GLenum subroutineInterface;
	//std::cout << "Program ID: " << m_id << "    Shader stage: ";
	if (shadertype == GL_VERTEX_SHADER) {
		programInterface = GL_VERTEX_SUBROUTINE_UNIFORM;
		subroutineInterface = GL_VERTEX_SUBROUTINE;
		//std::cout << "VERTEX\n";
	} else if (shadertype == GL_FRAGMENT_SHADER) {
		programInterface = GL_FRAGMENT_SUBROUTINE_UNIFORM;
		subroutineInterface = GL_FRAGMENT_SUBROUTINE;
		//std::cout << "FRAGMENT\n";
	} else if (shadertype == GL_GEOMETRY_SHADER) {
		programInterface = GL_GEOMETRY_SUBROUTINE_UNIFORM;
		subroutineInterface = GL_GEOMETRY_SUBROUTINE;
		//std::cout << "GEOMETRY\n";
	}

	// Get subroutine uniforms number for this shader stage
	int subUniNum;
	glGetProgramInterfaceiv(m_id, programInterface, GL_ACTIVE_RESOURCES, &subUniNum);
	//std::cout << "Subroutine Uniforms found: " << subUniNum << "\n";

	// Abort if no subroutine uniforms are found
	if (subUniNum <= 0) return;

	// Initialize string buffer for subroutine uniforms
	int maxSubUniNameLength;
	glGetProgramInterfaceiv(m_id, programInterface, GL_MAX_NAME_LENGTH, &maxSubUniNameLength);
	char *strbufSubUni = new char[maxSubUniNameLength];

	// Iterate through subroutine uniforms
	for (int i = 0; i < subUniNum; i++) {
		// Get name of subroutine uniform at index i
		glGetProgramResourceName(m_id, programInterface, i, maxSubUniNameLength, &len, strbufSubUni);
		std::pair<std::string, int> uniformPair = std::pair<std::string, int>{std::string{strbufSubUni}, i};
		if      (shadertype == GL_VERTEX_SHADER)   m_subroutineUniformsVertex.insert(uniformPair);
		else if (shadertype == GL_FRAGMENT_SHADER) m_subroutineUniformsFragment.insert(uniformPair);
		else if (shadertype == GL_GEOMETRY_SHADER) m_subroutineUniformsGeometry.insert(uniformPair);
		//std::cout << "Subroutine Uniform #" << i << ": " << std::string{strbufSubUni} << "\n";

		// Get number of subroutines for the subroutine uniform at index i
		int subNum;
		glGetProgramInterfaceiv(m_id, subroutineInterface, GL_ACTIVE_RESOURCES, &subNum);
		//std::cout << "Subroutines found: " << subNum << "\n";

		// Initialize string buffer for subroutine uniforms
		int maxSubNameLength;
		glGetProgramInterfaceiv(m_id, subroutineInterface, GL_MAX_NAME_LENGTH, &maxSubNameLength);
		char *strbufSub = new char[maxSubNameLength];
		
		// Get names of subroutines for the subroutine uniform at index i
		for (int j = 0; j < subNum; j++) {
			glGetProgramResourceName(m_id, subroutineInterface, j, maxSubNameLength, &len, strbufSub);
			std::pair<std::string, int> functionPair = std::pair<std::string, int>{std::string{strbufSub}, j};
			if      (shadertype == GL_VERTEX_SHADER)   m_subroutinesVertex.insert(functionPair);
			else if (shadertype == GL_FRAGMENT_SHADER) m_subroutinesFragment.insert(functionPair);
			else if (shadertype == GL_GEOMETRY_SHADER) m_subroutinesGeometry.insert(functionPair);
			//std::cout << "Subroutine #" << j << ": " << std::string{strbufSub} << "\n";
		}

		// Deallocate resources for subroutine functions
		delete[] strbufSub;
	}

	// Deallocate string buffer
	delete[] strbufSubUni;

	// Allocate resources for subroutines' configurations
	if      (shadertype == GL_VERTEX_SHADER)   m_subroutineConfigurationVertex = (unsigned int*) malloc(sizeof(unsigned int) * m_subroutinesVertex.size());
	else if (shadertype == GL_FRAGMENT_SHADER) m_subroutineConfigurationFragment = (unsigned int*) malloc(sizeof(unsigned int) * m_subroutinesFragment.size());
	else if (shadertype == GL_GEOMETRY_SHADER) m_subroutineConfigurationGeometry = (unsigned int*) malloc(sizeof(unsigned int) * m_subroutinesGeometry.size());
}

void Shader::setSubroutineUniform(GLenum shadertype, std::string uniformName, std::string subroutineName) {
	/*
	 *
	 * WARNING: NOT VERY SAFE! YOU NEED TO TWEAK THE CRAP OUT OF IT.
	 * 
	 */

	int uniformIndex;
	int subroutineIndex;
	switch(shadertype) {
		default:
			break;
		case GL_VERTEX_SHADER:
			uniformIndex = m_subroutineUniformsVertex.find(uniformName)->second;
			subroutineIndex = m_subroutinesVertex.find(subroutineName)->second;
			m_subroutineConfigurationVertex[uniformIndex] = subroutineIndex;
			break;
		case GL_FRAGMENT_SHADER:
			uniformIndex = m_subroutineUniformsFragment.find(uniformName)->second;
			subroutineIndex = m_subroutinesFragment.find(subroutineName)->second;
			m_subroutineConfigurationFragment[uniformIndex] = subroutineIndex;
			break;
		case GL_GEOMETRY_SHADER:
			uniformIndex = m_subroutineUniformsGeometry.find(uniformName)->second;
			subroutineIndex = m_subroutinesGeometry.find(subroutineName)->second;
			m_subroutineConfigurationGeometry[uniformIndex] = subroutineIndex;
			break;
	}
}

void Shader::loadSubroutines(GLenum shadertype) {
	unsigned int *config;
	GLsizei count;
	if (shadertype == GL_VERTEX_SHADER) {
		config = m_subroutineConfigurationVertex;
		count = m_subroutineUniformsVertex.size();
	} else if (shadertype == GL_FRAGMENT_SHADER) {
		config = m_subroutineConfigurationFragment;
		count = m_subroutineUniformsFragment.size();
	} else if (shadertype == GL_GEOMETRY_SHADER) {
		config = m_subroutineConfigurationGeometry;
		count = m_subroutineUniformsGeometry.size();
	}
	glUniformSubroutinesuiv(shadertype, count, (const GLuint*)config);
}

unsigned int Shader::getID() {
	return m_id;
}


// --- Private methods
void Shader::checkCompileErrors(unsigned int object, ShaderError type) {
	int success;
	char infoLog[1024];
	switch (type) {
		case SHADER_ERROR_VERTEX:
		case SHADER_ERROR_FRAGMENT:
		case SHADER_ERROR_GEOMETRY:
			glGetShaderiv(object, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(object, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER: compile-time error: type: " << printShaderError(type) << "\n" << infoLog << "\n";
			}
			break;
		
		case SHADER_ERROR_PROGRAM:
			glGetProgramiv(object, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(object, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER: link-time error: type: " << printShaderError(type) << "\n" << infoLog << "\n";
			}
			break;
	}
}

std::string Shader::printShaderError(ShaderError type) {
	std::string r;
	switch (type) {
		case SHADER_ERROR_VERTEX:
			r = "VERTEX";
			break;
			
		case SHADER_ERROR_FRAGMENT:
			r = "FRAGMENT";
			break;
			
		case SHADER_ERROR_GEOMETRY:
			r = "GEOMETRY";
			break;
		
		case SHADER_ERROR_PROGRAM:
			r = "PROGRAM";
			break;
	}
	
	return r;
}