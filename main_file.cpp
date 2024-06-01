
/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <chrono>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// #include "camera.cpp"
// #include "particles.cpp"
// #include "models.cpp"


// Camera settings
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraEye = glm::vec3(0, 1, -5);
glm::vec3 cameraPos = glm::vec3(0,0,0);


ShaderProgram* sp;
float fovy = 50.0f * PI / 180.0f;



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
    aiMesh* mesh = scene->mMeshes[meshIdx];
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


void draw_mesh_textured(const std::vector<MeshData> &mesh_vec, GLuint texture, GLint v0, ShaderProgram* sp1) {
  // sp->use();
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


ShaderProgram* sp_particles;


struct Particle {
  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec4 color;
  float size;
  float lifespan = -1.0f; // Remaining lifespan of the particle
  float distanceToCamera = -1.0f;

  bool operator<(const Particle &other) const {
    return other.distanceToCamera < this->distanceToCamera;
  }

  Particle() {
    position = glm::vec3(0);
    velocity = glm::vec3(0);
    color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    size = 20;
  }
};

const glm::float32 particle_box[] = {
    -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f,
};

const size_t PARTICLES_SIZE = 1000;

std::vector<Particle> particles = std::vector<Particle>(PARTICLES_SIZE);
size_t lastUsedParticle = 0;

GLuint billboard_vertex_buffer;
GLuint particles_position_buffer;
GLuint particles_size_buffer;
GLuint particles_color_buffer;

std::vector<glm::vec3> particles_position_data = std::vector<glm::vec3>(PARTICLES_SIZE); 
std::vector<glm::vec4> particles_color_data = std::vector<glm::vec4>(PARTICLES_SIZE); 
std::vector<GLubyte> particles_size_data = std::vector<GLubyte>(PARTICLES_SIZE); 

void particlesBuffersInit(){
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particle_box), particle_box, GL_STATIC_DRAW);

	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, PARTICLES_SIZE * 3 * sizeof(glm::float32), NULL, GL_STREAM_DRAW);

	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, PARTICLES_SIZE * sizeof(glm::float32), NULL, GL_STREAM_DRAW);

	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, PARTICLES_SIZE * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
}


size_t findUnusedParticle() {

  // ┌─┬─┬─┬─┬─┬─┐
  // │x│x│x│ │ │ │
  // └─┴─┴─┴─┴─┴─┘
  //      ^
  //      last used, move forward in the array
  for (size_t i = lastUsedParticle; i < particles.size(); i++) {
    if (particles[i].lifespan < 0) {
      lastUsedParticle = i;
      return i;
    }
  }

  // ┌─┬─┬─┬─┬─┬─┐
  // │ │ │x│x│x│x│
  // └─┴─┴─┴─┴─┴─┘
  //  ^         ^
  //  │        last used, move back to start
  //  │
  //  │         │
  //  └─────────┘

  for (size_t i = 0; i < lastUsedParticle; i++) {
    if (particles[i].lifespan < 0) {
      lastUsedParticle = i;
      return i;
    }
  }

  // No free particles
  return 0;
}

// Function to update particles
void updateParticles(float deltaTime) {
  int newparticles = (int)(deltaTime*5000.0);

  for (int i=0; i<newparticles; i++){
    size_t p = findUnusedParticle();
    particles[p].lifespan = 5.0f; // 5s
    particles[p].velocity = glm::vec3(0.0, 10.0f, 0.0f);
  }  

  size_t c = 0; //count
  for (auto& p : particles){
    if (p.lifespan < 0.0f){
      continue;
    }

    p.lifespan -= deltaTime;
  
    if (p.lifespan < 0.0f){
      p.distanceToCamera = -1.0f;
      c++;
      continue;
    }

    // velocity...
    p.position += p.velocity * deltaTime;

    p.distanceToCamera = glm::length(p.position - cameraPos);
    p.distanceToCamera *= p.distanceToCamera; // distance squared

    particles_position_data[c] = p.position;
    particles_size_data[c] = p.size;
    particles_color_data[c] = p.color;

    c++;
  }
}

// Function to render particles
void drawParticles(double deltaTime, GLint Texture, const glm::mat4& ViewMatrix, const glm::mat4& PV) {


  updateParticles(deltaTime);  

	std::sort(particles.begin(), particles.end());


	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, PARTICLES_SIZE * 3 * sizeof(glm::float32), NULL, GL_STREAM_DRAW); 
	glBufferSubData(GL_ARRAY_BUFFER, 0, PARTICLES_SIZE * sizeof(glm::float32) * 3, particle_box);

	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, PARTICLES_SIZE * 4 * sizeof(glm::float32), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, PARTICLES_SIZE * sizeof(glm::float32), particle_box);

	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, PARTICLES_SIZE * 4 * sizeof(glm::float32), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, PARTICLES_SIZE * sizeof(glm::float32) * 4, particle_box);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Use our shader
	// glUseProgram(programID);
  
  sp_particles->use();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);
	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(sp_particles->u("particle_tex"), 0);

	// Same as the billboards tutorial

	
	glUniform3f(sp_particles->u("camera_right"), ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
	glUniform3f(sp_particles->u("camera_up"), ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

	glUniformMatrix4fv(sp_particles->u("PV"), 1, GL_FALSE, glm::value_ptr(PV));

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	
	// 2nd attribute buffer : positions of particles' centers
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		3,                                // size : x + y + z +  => 3
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, particles_size_buffer);
	glVertexAttribPointer(
		2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		1,                                // size 
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// 3rd attribute buffer : particles' colors
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glVertexAttribPointer(
		3,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size : r + g + b + a => 4
		GL_FLOAT,                 // type
		GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
	glVertexAttribDivisor(2, 1); // size: one per quad (its center)                 -> 1
	glVertexAttribDivisor(3, 1); // color : one per quad                                  -> 1

	// Draw the particles !
	// This draws many times a small triangle_strip (which looks like a quad).
	// This is equivalent to :
	// for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
	// but faster.
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, PARTICLES_SIZE);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

}



float speed_x = 0;
float speed_y = 0;
float aspectRatio = 1;


bool firstMouse = true;

std::chrono::steady_clock::time_point lastUpdateTime =
    std::chrono::steady_clock::now();




// Procedura obsługi błędów
void error_callback(int error, const char *description) {
  fputs(description, stderr);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action,int mods) {
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_LEFT)
      speed_x = -PI / 2;
    if (key == GLFW_KEY_RIGHT)
      speed_x = PI / 2;
    if (key == GLFW_KEY_UP)
      speed_y = PI / 2;
    if (key == GLFW_KEY_DOWN)
      speed_y = -PI / 2;
  }
  if (action == GLFW_RELEASE) {
    if (key == GLFW_KEY_LEFT)
      speed_x = 0;
    if (key == GLFW_KEY_RIGHT)
      speed_x = 0;
    if (key == GLFW_KEY_UP)
      speed_y = 0;
    if (key == GLFW_KEY_DOWN)
      speed_y = 0;
  }
}

void windowResizeCallback(GLFWwindow *window, int width, int height) {
  if (height == 0)
    return;
  aspectRatio = (float)width / (float)height;
  glViewport(0, 0, width, height);
}

GLuint readTexture(const char *filename) {
  GLuint tex;
  glActiveTexture(GL_TEXTURE0);

  // Wczytanie do pamięci komputera
  std::vector<unsigned char> image; // Alokuj wektor do wczytania obrazka
  unsigned width, height; // Zmienne do których wczytamy wymiary obrazka
  // Wczytaj obrazek
  unsigned error = lodepng::decode(image, width, height, filename);

  // Import do pamięci karty graficznej
  glGenTextures(1, &tex);            // Zainicjuj jeden uchwyt
  glBindTexture(GL_TEXTURE_2D, tex); // Uaktywnij uchwyt
  // Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
  glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               (unsigned char *)image.data());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return tex;
}

// Procedura inicjująca
void initOpenGLProgram(GLFWwindow *window) {
  //************Tutaj umieszczaj kod, który należy wykonać raz, na początku
  // programu************
  glClearColor(0.1, 0.37, 0.37, 1);
  glEnable(GL_DEPTH_TEST);
  // glDepthFunc(GL_LESS);
  glfwSetWindowSizeCallback(window, windowResizeCallback);
  glfwSetKeyCallback(window, keyCallback);

  sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");
  // sp_particles = new ShaderProgram("v_particles.glsl", NULL, "f_particles.glsl");

  texWulkan = readTexture("Wulkan_ColorMap.png");
  texLava = readTexture("mlawa.png");
  texNiebo = readTexture("sky.png");
  texRex = readTexture("trex_diff.png");
  texTree = readTexture("Ramas Nieve.png");

  loadModel("wulkan.fbx", meshes_vulkan);
  loadModel("lava.fbx", meshes_lava);
  loadModel("floor.fbx", meshes_floor);
  loadModel("trex.fbx", meshes_trex);
  loadModel("SnowTree.fbx", meshes_tree);

  particlesBuffersInit();
  
}

// Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow *window) {
  //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************

  delete sp;
  delete sp_particles;

  glDeleteTextures(1, &texWulkan);
  glDeleteTextures(1, &texLava);
  glDeleteTextures(1, &texNiebo);
  glDeleteTextures(1, &texRex);
  glDeleteTextures(1, &texTree);

  // TODO: delete particle textures? // TODO: unload models?
}

// Procedura rysująca zawartość sceny
void drawScene(GLFWwindow *window, float angle_x, float angle_y,double deltaTime) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 V = glm::lookAt(glm::vec3(0,1,-5), glm::vec3(0, 0, 0),glm::vec3(0,1,0));

  glm::mat4 P = glm::perspective(fovy, aspectRatio, 0.1f, 50.0f);

  glm::mat4 M = glm::mat4(1.0f);
  // M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(1.0f, 0, 0));
  M = glm::rotate(M, angle_y, glm::vec3(1.0f, 0.0f, 0.0f));
  M = glm::rotate(M, angle_x, glm::vec3(0.0f, 0.0f, 1.0f));


  sp->use();
  glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
  glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
  glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

  glUniform1i(sp->u("textureMap1"), 1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texNiebo);

  draw_mesh_textured(meshes_floor, texWulkan, 0, sp);

  // glm::mat4 Mlava = glm::scale(M, glm::vec3(2.01));
  glm::mat4 Mlava = M;
  glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mlava));
  draw_mesh_textured(meshes_lava, texLava, 0, sp);

  // glm::mat4 Mvolcano = glm::scale(M, glm::vec3(2));
  glm::mat4 Mvolcano = M;
  glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mvolcano));
  draw_mesh_textured(meshes_vulkan, texWulkan, 0, sp);

  // glm::mat4 Mtrex = glm::translate(M, glm::vec3(2.0f, 1, 0.1));
  glm::mat4 Mtrex = M;
  // Mtrex = glm::rotate(Mtrex, glm::radians(180.0f), glm::vec3(0, 1, 1));
  // Mtrex = glm::rotate(Mtrex, glm::radians(90.0f), glm::vec3(0, 1, 0));
  // Mtrex = glm::scale(Mtrex, glm::vec3(0.1));
  glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mtrex));
  draw_mesh_textured(meshes_trex, texRex, 0, sp);

  for (const auto &tree : treepos) {
    // glm::mat4 Mtree = glm::translate(M, tree);
    glm::mat4 Mtree = M;
    // Mtree = glm::scale(Mtree, glm::vec3(0.05f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mtree));
    draw_mesh_textured(meshes_tree, texTree, 0, sp);
  }

  // drawParticles(deltaTime,texNiebo,V,P*V);
}

int main(void) {
  GLFWwindow *window; // Wskaźnik na obiekt reprezentujący okno

  glfwSetErrorCallback(error_callback); // Zarejestruj procedurę obsługi błędów

  if (!glfwInit()) { // Zainicjuj bibliotekę GLFW
    fprintf(stderr, "Nie można zainicjować GLFW.\n");
    exit(EXIT_FAILURE);
  }

  window = glfwCreateWindow(
      1000, 1000, "OpenGL", NULL,
      NULL); // Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

  if (!window) // Jeżeli okna nie udało się utworzyć, to zamknij program
  {
    fprintf(stderr, "Nie można utworzyć okna.\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(
      window); // Od tego momentu kontekst okna staje się aktywny i polecenia
               // OpenGL będą dotyczyć właśnie jego.
  glfwSwapInterval(
      1); // Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

  if (glewInit() != GLEW_OK) { // Zainicjuj bibliotekę GLEW
    fprintf(stderr, "Nie można zainicjować GLEW.\n");
    exit(EXIT_FAILURE);
  }

  initOpenGLProgram(window); // Operacje inicjujące

  // Główna pętla
  float angle_x = 0; // Aktualny kąt obrotu obiektu
  float angle_y = 0; // Aktualny kąt obrotu obiektu
  // glfwSetTime(0); //Zeruj timer
  while (!glfwWindowShouldClose(window)) {
    angle_x += speed_x * glfwGetTime(); // Update rotation angle
    angle_y += speed_y * glfwGetTime(); // Update rotation angle
    std::chrono::steady_clock::time_point currentTime =
        std::chrono::steady_clock::now();
    std::chrono::duration<float> deltaTimeDuration =
        currentTime - lastUpdateTime;
    double deltaTime = deltaTimeDuration.count(); // Convert duration to seconds
    lastUpdateTime = currentTime;
    glfwSetTime(0);

    // Render scene
    drawScene(window, angle_x, angle_y, deltaTime);

    glfwSwapBuffers(window); // Swap buffers
    glfwPollEvents();        // Process events
  }

  freeOpenGLProgram(window);

  glfwDestroyWindow(window); // Usuń kontekst OpenGL i okno
  glfwTerminate();           // Zwolnij zasoby zajęte przez GLFW
  exit(EXIT_SUCCESS);
}
