#ifndef MESH_HPP
#define MESH_HPP


#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>


// --- Vertex data structure
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

// --- Mesh class
class Mesh {
    public:
        // --- Constructors, destructors and operator overloadings
        // Delete copy constructor and copy assignment
        Mesh(const Mesh& copy) = delete; //disallow copy
        Mesh& operator=(const Mesh &) = delete;
        
        // This constructor empties the source vectors (vertices and indices)
        Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices) noexcept;

        // Move constructor
        Mesh(Mesh &&move) noexcept;

        // Move assignment
        Mesh &operator=(Mesh &&move) noexcept;

        // Destructor
        ~Mesh() noexcept;

        // --- Public methods
        GLuint getVAO();
        int getIndicesNumber();

    private:
        // --- Private members
        std::vector<Vertex> m_vertices;
        std::vector<GLuint> m_indices;
        GLuint m_VAO;
        GLuint m_VBO;
        GLuint m_EBO;

        // --- Private methods
        void setup();
        void freeGPUresources();
};


#endif // MESH_H