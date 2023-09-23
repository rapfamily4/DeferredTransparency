#ifndef SHADER_HPP
#define SHADER_HPP

#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


/* Copilation and liking error types */
enum ShaderError {
	SHADER_ERROR_VERTEX,
	SHADER_ERROR_FRAGMENT,
	SHADER_ERROR_GEOMETRY,
	SHADER_ERROR_PROGRAM
};

/* Shader class with utilities */
class Shader {
	public:		
		// Constructor and destructor
		Shader();
		~Shader();
		
		// Sets the shader as active
		Shader *use();
		
		// Compiles the shader with the given source code
		void compile(const char *vertexSource, const char *fragmentSource, const char *geometrySource = nullptr);
		
		// Uniform setters
		void setFloat(const char *name, float value);
		void setInteger(const char *name, int value);
		void setVector2(const char *name, float x, float y);
		void setVector2(const char *name, const glm::vec2 &value);
		void setVector3(const char *name, float x, float y, float z);
		void setVector3(const char *name, const glm::vec3 &value);
		void setVector4(const char *name, float x, float y, float z, float w);
		void setVector4(const char *name, const glm::vec4 &value);
		void setMatrix3(const char *name, const glm::mat3 &matrix);
		void setMatrix4(const char *name, const glm::mat4 &matrix);
		
		// Subroutine management
		void setupSubroutines(GLenum shadertype);
		void setSubroutineUniform(GLenum shadertype, std::string uniformName, std::string subroutineName);
		void loadSubroutines(GLenum shadertype);

		// Getters
		unsigned int getID();
	
	private:
		// --- Private members
		// Program ID
		unsigned int m_id;

		// Maps of subroutine uniforms, matching names with their index.
		std::map<std::string, unsigned int> m_subroutineUniformsVertex;
		std::map<std::string, unsigned int> m_subroutineUniformsFragment;
		std::map<std::string, unsigned int> m_subroutineUniformsGeometry;

		// Maps of subroutines, matching names with their index.
		std::map<std::string, unsigned int> m_subroutinesVertex;
		std::map<std::string, unsigned int> m_subroutinesFragment;
		std::map<std::string, unsigned int> m_subroutinesGeometry;

		// Current configurations of subroutines.
		unsigned int *m_subroutineConfigurationVertex;
		unsigned int *m_subroutineConfigurationFragment;
		unsigned int *m_subroutineConfigurationGeometry;
		
		// --- Private methods
		void checkCompileErrors(unsigned int object, ShaderError type);
		std::string printShaderError(ShaderError type);
};


#endif