
#include "shaderprogram.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <stdlib.h>

#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


const std::vector<glm::vec3> treepos = {
    {2, 4, 0.02},       {2.1, 4, 0.02},      {2.2, 4.1, 0.02},
    {2.1, 4.05, 0.02},  {2.4, 3.8, 0.02},    {0.5, 2.7, 0.02},
    {0.5, 2.8, 0.02},   {0.4, 2.7, 0.02},    {0.3, 2.9, 0.02},
    {-2, -3, 0.06},     {-2.1, -3.33, 0.06}, {-2.07, -3.5, 0.06},
    {-2.2, -3.9, 0.06}, {-1.8, -3.3, 0.05},  {-2.4, -3.2, 0.05},
    {-3.3, 2.2, 0.06},
};

struct MeshData {
  std::vector<glm::vec4> vertices;
  std::vector<glm::vec4> normals;
  std::vector<glm::vec2> texCoords;
  std::vector<unsigned int> indices;
};

std::vector<MeshData> meshes_vulkan;
std::vector<MeshData> meshes_lava;
std::vector<MeshData> meshes_floor;
std::vector<MeshData> meshes_trex;
std::vector<MeshData> meshes_tree;

GLuint texWulkan;
GLuint texLava;
GLuint texNiebo;
GLuint texRex;
GLuint texTree;

void loadModel(std::string plik, std::vector<MeshData> &meshContainer) {
  using namespace std;

  Assimp::Importer importer;
  const aiScene* scene = 
      importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

  if (!scene) {
    cerr << "Error loading model: " << importer.GetErrorString() << endl;
    return;
  }

  for (unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx) {
    auto& mesh = scene->mMeshes[meshIdx];
    MeshData meshData;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
      aiVector3D vertex = mesh->mVertices[i];
      meshData.vertices.push_back(
          glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f));

      aiVector3D normal = mesh->mNormals[i];
      meshData.normals.push_back(glm::vec4(normal.x, normal.y, normal.z, 0.0f));

      if (mesh->mTextureCoords[0]) {
        aiVector3D texCoord = mesh->mTextureCoords[0][i];
        meshData.texCoords.push_back(glm::vec2(texCoord.x, texCoord.y));
      } else {
        meshData.texCoords.push_back(glm::vec2(0.0f, 0.0f));
      }
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
      aiFace &face = mesh->mFaces[i];
      for (unsigned int j = 0; j < face.mNumIndices; j++) {
        meshData.indices.push_back(face.mIndices[j]);
      }
    }

    meshContainer.push_back(meshData);
  }

  cout << "Model loaded!" << endl;
}


void draw_mesh_textured(const std::vector<MeshData> &mesh_vec, GLuint texture, GLint v0, ShaderProgram* sp) {
  glUniform1i(sp->u("textureMap0"), v0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);

  // Draw first model
  for (const MeshData &mesh : mesh_vec) {
    glEnableVertexAttribArray(sp->a("vertex"));
    glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0,
                          mesh.vertices.data());

    glEnableVertexAttribArray(sp->a("normal"));
    glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0,
                          mesh.normals.data());

    glEnableVertexAttribArray(sp->a("texCoord0"));
    glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0,
                          mesh.texCoords.data());

    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT,
                   mesh.indices.data());

    glDisableVertexAttribArray(sp->a("vertex"));
    glDisableVertexAttribArray(sp->a("normal"));
    glDisableVertexAttribArray(sp->a("texCoord0"));
  }
}

