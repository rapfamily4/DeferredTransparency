#ifndef MODEL_HPP
#define MODEL_HPP


#include <iostream>

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "resources/mesh.hpp"


// --- Model class
class Model {
    public:
        // --- Constructors, destructors and operator overloadings
        // Delete copy constructor and copy assignment
        Model(const Model& model) = delete; //disallow copy
        Model& operator=(const Model& copy) = delete;

        // Default move constructor and move assignment
        Model& operator=(Model&& move) noexcept = default;
        Model(Model&& model) = default;

        // Constructor
        Model();

        // --- Public methods
        void setup(const aiScene *scene);
        void buildMeshLists(unsigned int *vaoList, int *indicesNumberList);
        int getMeshesNumber();

private:
    // --- Private members
    std::vector<Mesh> m_meshes;

    // --- Private methods
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh);
};


#endif // MODEL_HPP