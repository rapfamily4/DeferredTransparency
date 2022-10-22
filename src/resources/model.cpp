#include <iostream>

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "resources/mesh.hpp"
#include "resources/model.hpp"


// --- Public constructor
Model::Model() {}


// --- Public methods
void Model::setup(const aiScene *scene) {
    // Begin the recursive processing of nodes in the Assimp data structure
    this->processNode(scene->mRootNode, scene);
}

void Model::buildMeshLists(unsigned int *vaoList, int *indicesNumberList) {
    for(unsigned int i = 0; i < m_meshes.size(); i++) {
        vaoList[i] = m_meshes[i].getVAO();
        indicesNumberList[i] = m_meshes[i].getIndicesNumber();
    }
}

int Model::getMeshesNumber() {
    return m_meshes.size();
}


// --- Private methods
void Model::processNode(aiNode* node, const aiScene* scene) {
    // Process each mesh inside the current node
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        this->m_meshes.emplace_back(processMesh(mesh));
    }

    // Recursively process each of the children nodes
    for(unsigned int i = 0; i < node->mNumChildren; i++)
        this->processNode(node->mChildren[i], scene);
}

Mesh Model::processMesh(aiMesh* mesh) {
    // Data structures for vertices and indices of vertices (for faces)
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        // The vector data type used by Assimp is different than the GLM vector needed to allocate the OpenGL buffers
        // I need to convert the data structures (from Assimp to GLM, which are fully compatible to the OpenGL)
        glm::vec3 vector;
        // vertices coordinates
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        // Normals
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;
        // Texture Coordinates
        // if the model has texture coordinates, than we assign them to a GLM data structure, otherwise we set them at 0
        // if texture coordinates are present, than Assimp can calculate tangents and bitangents, otherwise we set them at 0 too
        if(mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            // in this example we assume the model has only one set of texture coordinates. Actually, a vertex can have up to 8 different texture coordinates. For other models and formats, this code needs to be adapted and modified.
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;

            // Tangents
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;
            // Bitangents
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.bitangent = vector;
        } else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
            std::cout << "WARNING::ASSIMP:: MODEL WITHOUT UV COORDINATES -> TANGENT AND BITANGENT ARE = 0\n";
        }

        // Add the vertex to the list
        vertices.emplace_back(vertex);
    }

    // For each face of the mesh, we retrieve the indices of its vertices, and we store them in a vector data structure
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.emplace_back(face.mIndices[j]);
    }

    // Return an instance of the Mesh class created using the vertices and faces data structures created above.
    return Mesh(vertices, indices);
}