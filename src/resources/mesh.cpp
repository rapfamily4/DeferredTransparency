#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "resources/mesh.hpp"

// --- Public constructors, destructors and operator overloadings
// We use initializer list and std::move in order to avoid a copy of the arguments
// This constructor empties the source vectors (vertices and indices)
Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices) noexcept :
    m_vertices{ std::move(vertices) },
    m_indices{ std::move(indices) } {
    // ---
    setup();
}

// Move constructor
// The source object of a move constructor is not expected to be valid after the move.
// In our case it will no longer imply ownership of the GPU resources and its vectors will be empty.
Mesh::Mesh(Mesh &&move) noexcept :
    m_vertices{ std::move(move.m_vertices) },
    m_indices{ std::move(move.m_indices) },
    m_VAO{ move.m_VAO },
    m_VBO{ move.m_VBO },
    m_EBO{ move.m_EBO } {
    // ---
    // We *could* set VBO and EBO to 0 too, but since we bring all the 3 values around we can use just
    // one of them to check ownership of the 3 resources.
    move.m_VAO = 0;
}

// Move assignment
Mesh &Mesh::operator=(Mesh &&move) noexcept {
    // Calls the function which will delete (if needed) the GPU resources for this instance
    freeGPUresources();

    // If source instance has GPU resources...
    if (move.m_VAO) {
        m_vertices = std::move(move.m_vertices);
        m_indices = std::move(move.m_indices);
        m_VAO = move.m_VAO;
        m_VBO = move.m_VBO;
        m_EBO = move.m_EBO;

        move.m_VAO = 0;
    } else { // ... else, source instance was already invalid
        
        m_VAO = 0;
    }
    return *this;
}

// Destructor
Mesh::~Mesh() noexcept {
    // Calls the function which will delete (if needed) the GPU resources
    freeGPUresources();
}


// --- Public methods
GLuint Mesh::getVAO() {
    return m_VAO;
}

int Mesh::getIndicesNumber() {
    return m_indices.size();
}


// --- Private methods
void Mesh::setup() {
    // we create the buffers
    glGenVertexArrays(1, &this->m_VAO);
    glGenBuffers(1, &this->m_VBO);
    glGenBuffers(1, &this->m_EBO);

    // VAO is made "active"
    glBindVertexArray(this->m_VAO);
    // we copy data in the VBO - we must set the data dimension, and the pointer to the structure cointaining the data
    glBindBuffer(GL_ARRAY_BUFFER, this->m_VBO);
    glBufferData(GL_ARRAY_BUFFER, this->m_vertices.size() * sizeof(Vertex), &this->m_vertices[0], GL_STATIC_DRAW);
    // we copy data in the EBO - we must set the data dimension, and the pointer to the structure cointaining the data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->m_indices.size() * sizeof(GLuint), &this->m_indices[0], GL_STATIC_DRAW);

    // we set in the VAO the pointers to the different vertex attributes (with the relative offsets inside the data structure)
    // vertex positions
    // these will be the positions to use in the layout qualifiers in the shaders ("layout (location = ...)"")
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
    // Texture Coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoords));
    // Tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, tangent));
    // Bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, bitangent));

    glBindVertexArray(0);
}

void Mesh::freeGPUresources() {
    // If VAO is 0, this instance of Mesh has been through a move, and no longer owns GPU resources,
    // so there's no need for deleting.
    if (m_VAO) {
        glDeleteVertexArrays(1, &this->m_VAO);
        glDeleteBuffers(1, &this->m_VBO);
        glDeleteBuffers(1, &this->m_EBO);
    }
}