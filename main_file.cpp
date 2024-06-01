
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

#pragma once
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
#include <memory>
#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "particles.cpp"
#include "models.cpp"

float speed_x = 0;
float speed_y = 0;
float aspectRatio = 1;

// Camera settings
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400.0f, lastY = 300.0f;
float fov = 45.0f;

std::chrono::steady_clock::time_point lastUpdateTime =
    std::chrono::steady_clock::now();

std::shared_ptr<ShaderProgram> sp;


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
  glDepthFunc(GL_LESS);
  glfwSetWindowSizeCallback(window, windowResizeCallback);
  glfwSetKeyCallback(window, keyCallback);

  sp = std::make_shared<ShaderProgram>(
      ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl"));
  sp_particles = std::make_shared<ShaderProgram>(
      ShaderProgram("v_particles.glsl", NULL, "f_particles.glsl"));

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

  // TODO: particle and billboard buffers???
}

// Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow *window) {
  //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************

  glDeleteTextures(1, &texWulkan);
  glDeleteTextures(1, &texLava);
  glDeleteTextures(1, &texNiebo);
  glDeleteTextures(1, &texRex);
  glDeleteTextures(1, &texTree);

  // TODO: delete particle textures?
  // TODO: unload models?
}

// Procedura rysująca zawartość sceny
void drawScene(GLFWwindow *window, float angle_x, float angle_y,
               double deltaTime) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 V = glm::lookAt(glm::vec3(0, 1, -5), glm::vec3(0, 0, 0),
                            glm::vec3(0.0f, 1.0f, 0.0f));

  glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, aspectRatio, 0.1f, 50.0f);

  glm::mat4 M = glm::mat4(1.0f);
  M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(1.0f, 0, 0));
  M = glm::rotate(M, angle_y, glm::vec3(1.0f, 0.0f, 0.0f));
  M = glm::rotate(M, angle_x, glm::vec3(0.0f, 0.0f, 1.0f));

  sp->use();
  glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
  glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
  glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

  glUniform1i(sp->u("textureMap1"), 1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texNiebo);

  // draw_mesh_textured(meshes_floor, texWulkan, 0, sp);

  // glm::mat4 Mlava = glm::scale(M, glm::vec3(2.01));
  // glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mlava));
  // draw_mesh_textured(meshes_lava, texLava, 0, sp);

  // glm::mat4 Mvolcano = glm::scale(M, glm::vec3(2));
  // glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mvolcano));
  // draw_mesh_textured(meshes_vulkan, texWulkan, 0, sp);

  // glm::mat4 Mtrex = glm::translate(M, glm::vec3(2.0f, 1, 0.1));
  // Mtrex = glm::rotate(Mtrex, glm::radians(180.0f), glm::vec3(0, 1, 1));
  // Mtrex = glm::rotate(Mtrex, glm::radians(90.0f), glm::vec3(0, 1, 0));
  // Mtrex = glm::scale(Mtrex, glm::vec3(0.1));
  // glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mtrex));
  // draw_mesh_textured(meshes_trex, texRex, 0, sp);

  // for (const auto &tree : treepos) {
  //   glm::mat4 Mtree = glm::translate(M, tree);
  //   Mtree = glm::scale(Mtree, glm::vec3(0.05f));
  //   glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mtree));
  //   draw_mesh_textured(meshes_tree, texTree, 0, sp);
  // }

  drawParticles(deltaTime);
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
    // angle_x += speed_x * glfwGetTime(); // Update rotation angle
    // angle_y += speed_y * glfwGetTime(); // Update rotation angle
    std::chrono::steady_clock::time_point currentTime =
        std::chrono::steady_clock::now();
    std::chrono::duration<float> deltaTimeDuration =
        currentTime - lastUpdateTime;
    double deltaTime = deltaTimeDuration.count(); // Convert duration to seconds
    lastUpdateTime = currentTime;
    glfwSetTime(0); // Reset timer

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
