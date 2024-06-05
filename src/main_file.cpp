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

#include <glm/ext/matrix_transform.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include <algorithm>
#include <chrono>

#include <iostream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

ShaderProgram* sp;

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
std::vector<MeshData> meshes_kostka;

GLuint texWulkan;
GLuint specWulkan;

GLuint texLava;
GLuint specLava;
// GLuint texLavaLight;

GLuint texNiebo;

GLuint texRex;
GLuint specRex;

GLuint texTree;
GLuint specTree;

GLuint texKostka;
GLuint specKostka;

float random_blysk = 0;
const double c_okres_blysku = 0.5f; // s
const double c_czas_blysku = 0.2; //s 
double okres_blysku = c_okres_blysku; // s
double czas_blysku = c_czas_blysku; //s 

const GLfloat lava_tex_coords_offset_per_frame = 0.001f;
int tex_offset_counter = 0; 

void loadModel(std::string plik, std::vector<MeshData>& meshContainer)
{
	using namespace std;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
	if (!scene) {
		cerr << "Error loading model: " << plik << "\n" << importer.GetErrorString() << endl;
		return;
	}

	for (unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx) {
		aiMesh* mesh = scene->mMeshes[meshIdx];
		MeshData meshData;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			aiVector3D vertex = mesh->mVertices[i];
			meshData.vertices.push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f));

			aiVector3D normal = mesh->mNormals[i];
			meshData.normals.push_back(glm::vec4(normal.x, normal.y, normal.z, 0.0f));

			if (mesh->mTextureCoords[0]) {
				aiVector3D texCoord = mesh->mTextureCoords[0][i];
				meshData.texCoords.push_back(glm::vec2(texCoord.x, texCoord.y));
			}
			else {
				meshData.texCoords.push_back(glm::vec2(0.0f, 0.0f));
			}
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace& face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				meshData.indices.push_back(face.mIndices[j]);
			}
		}

		meshContainer.push_back(meshData);
	}

	cout << "Model loaded!" << endl;
}

void draw_mesh(const std::vector<MeshData>& mesh_vec){
	// Draw first model
	for (const MeshData& mesh : mesh_vec) {
		glEnableVertexAttribArray(sp->a("vertex"));
		glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, mesh.vertices.data());

		glEnableVertexAttribArray(sp->a("normal"));
		glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, mesh.normals.data());

		glEnableVertexAttribArray(sp->a("texCoord0"));
		glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, mesh.texCoords.data());

		glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, mesh.indices.data());

		glDisableVertexAttribArray(sp->a("vertex"));
		glDisableVertexAttribArray(sp->a("normal"));
		glDisableVertexAttribArray(sp->a("texCoord0"));
	}
}

void draw_mesh_textured(const std::vector<MeshData>& mesh_vec, GLuint texture, GLuint specular,  GLint v0) {
	glUniform1i(sp->u("textureMap0"), v0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(sp->u("textureMap2"), v0+2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, specular);
	draw_mesh(mesh_vec);
}

float speed_x = 0;
float speed_y = 0;
float aspectRatio = 1;

// Camera settings


glm::vec3 cameraPos = glm::vec3(0, 1, -5);
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400.0f, lastY = 300.0f;
float fov = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
std::chrono::steady_clock::time_point lastUpdateTime =
std::chrono::steady_clock::now();


const glm::vec3 start_pos_particle = glm::vec3(0, 0, 1);

struct Particle {
	glm::vec3 position;
	glm::vec3 velocity;
	float lifespan = -1.0f; // Remaining lifespan of the particle
	float distanceToCamera = -1.0f;

	bool operator<(const Particle& other) const {
		return other.distanceToCamera < this->distanceToCamera;
	}

	Particle() {
		position = start_pos_particle;
		velocity = glm::vec3(0);
	}
};

const size_t PARTICLES_SIZE = 300;
std::vector<Particle> particles = std::vector<Particle>(PARTICLES_SIZE);
size_t lastUsedParticle = 0;
std::vector<glm::vec3> particles_position_data = std::vector<glm::vec3>(PARTICLES_SIZE);

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
	int newparticles = 10;

	float spread = 0.5f;
	glm::vec3 maindir = glm::vec3(0.0f, 0.0f, 1.0f);
	// Very bad way to generate a random direction; 
	// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
	// combined with some user-controlled parameters (main direction, spread, etc)
	


	// Very bad way to generate a random color
	//ParticlesContainer[particleIndex].r = rand() % 256;
	//ParticlesContainer[particleIndex].g = rand() % 256;
	//ParticlesContainer[particleIndex].b = rand() % 256;
	//ParticlesContainer[particleIndex].a = (rand() % 256) / 3;

	//ParticlesContainer[particleIndex].size = (rand() % 1000) / 2000.0f + 0.1f;

	for (int i = 0; i < newparticles; i++) {
		size_t p = findUnusedParticle();
		particles[p].lifespan = 3.0f; 
		glm::vec3 randomdir = glm::vec3(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
		);

		particles[p].velocity = maindir + randomdir * spread;
	}

	size_t c = 0; //count
	for (auto& p : particles) {
		if (p.lifespan < 0.0f) {
			p.position = start_pos_particle;
			continue;
		}

		p.lifespan -= deltaTime;
		// std::cout << p.lifespan << std::endl;

		if (p.lifespan < 0.0f) {
			p.position = start_pos_particle;
			p.distanceToCamera = -1.0f;
			c++;
			continue;
		}

		// velocity...
		p.position += p.velocity * deltaTime;

		p.distanceToCamera = glm::length(p.position - cameraPos);
		p.distanceToCamera *= p.distanceToCamera; // distance squared

		particles_position_data[c] = p.position;
		c++;
	}
}

void drawParticles(double deltaTime, const glm::mat4& M) {

	updateParticles(deltaTime);

	std::sort(particles.begin(), particles.end());

	for (auto& p : particles_position_data) {
		glm::mat4 Mkostka = glm::translate(M, p);
		Mkostka = glm::scale(Mkostka, glm::vec3(0.02));
		// std::cout << Mkostka[0][0] << " " << Mkostka[1][1] << " " << Mkostka[2][2] << " "<< Mkostka[3][3] << std::endl;
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mkostka));
		draw_mesh_textured(meshes_kostka, texKostka, specKostka, 0);
	}

}

const glm::vec3 treepos[] = {
	{2,4,0.02},
	{2.1,4,0.02},
	{2.2,4.1,0.02},
	{2.1,4.05,0.02},
	{2.4,3.8,0.02},
	{0.5,2.7,0.02},
	{0.5,2.8,0.02},
	{0.4,2.7,0.02},
	{0.3,2.9,0.02},
	{-2,-3,0.06},
	{-2.1,-3.33,0.06},
	{-2.07,-3.5,0.06},
	{-2.2,-3.9,0.06},
	{-1.8,-3.3,0.05},
	{-2.4,-3.2,0.05},
	{-3.3,2.2,0.06},
};

//Odkomentuj, żeby rysować kostkę
//float* vertices = myCubeVertices;
//float* normals = myCubeNormals;
//float* texCoords = myCubeTexCoords;
//float* colors = myCubeColors;
//int vertexCount = myCubeVertexCount;


////Odkomentuj, żeby rysować czajnik
//float* vertices = myTeapotVertices;
//float* normals = myTeapotVertexNormals;
//float* texCoords = myTeapotTexCoords;
// float* colors = myTeapotColors;
//int vertexCount = myTeapotVertexCount;





//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_x = -PI / 2;
		if (key == GLFW_KEY_RIGHT) speed_x = PI / 2;
		if (key == GLFW_KEY_UP) speed_y = PI / 2;
		if (key == GLFW_KEY_DOWN) speed_y = -PI / 2;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_x = 0;
		if (key == GLFW_KEY_RIGHT) speed_x = 0;
		if (key == GLFW_KEY_UP) speed_y = 0;
		if (key == GLFW_KEY_DOWN) speed_y = 0;
	}
}


void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0.1, 0.37, 0.37, 1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");

	texWulkan = readTexture("../assets/textures/Wulkan_ColorMap.png");
	specWulkan = readTexture("../assets/textures/Wulkan_specular.png");
	
	texLava = readTexture("../assets/textures/mlawa.png");
	specLava = readTexture("../assets/textures/mlawa_specular.png");
	// texLavaLight = readTexture("../assets/textures/mlawa_lightmap.png");

	texNiebo = readTexture("../assets/textures/sky.png");

	texRex = readTexture("../assets/textures/trex_diff.png");
	specRex = readTexture("../assets/textures/trex_specular.png");
	
	texTree = readTexture("../assets/textures/Ramas Nieve.png");
	specTree = readTexture("../assets/textures/tree_specular.png");

	texKostka = readTexture("../assets/textures/mlawa_lightmap.png");
	specKostka = specLava;
	

	loadModel("../assets/models/wulkan.fbx", meshes_vulkan);
	loadModel("../assets/models/lava.fbx", meshes_lava);
	loadModel("../assets/models/floor.fbx", meshes_floor);
	loadModel("../assets/models/trex.fbx", meshes_trex);
	loadModel("../assets/models/SnowTree.fbx", meshes_tree);
	loadModel("../assets/models/particle.fbx", meshes_kostka);
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************

	delete sp;

	glDeleteTextures(1, &texWulkan);
}



//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float angle_x, float angle_y, double deltaTime) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 V = glm::lookAt(
		cameraPos,
		glm::vec3(0, 0, 0),
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
	glUniform2f(sp->u("textureRoll"), 0,0);

	glUniform1i(sp->u("textureMap1"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texNiebo);


	if (okres_blysku < 0) {
		okres_blysku= c_okres_blysku*(rand()%5)/5.0f + 1.0f;
		random_blysk = (rand()%5)/5.0f;
	} else {
		if (czas_blysku < 0){
			czas_blysku= c_czas_blysku;
			random_blysk = 0;
		} else {
			czas_blysku -= deltaTime;
		}
		okres_blysku -= deltaTime;
	} 
	
	glUniform1f(sp->u("random"), random_blysk);

	draw_mesh_textured(meshes_floor, texWulkan, specWulkan, 0);

	glm::mat4 Mlava = glm::scale(M, glm::vec3(2.01)); // Adjust position if needed
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mlava));

	glUniform2f(sp->u("textureRoll"), 0,tex_offset_counter*lava_tex_coords_offset_per_frame);
	tex_offset_counter = std::max(tex_offset_counter+1, 60);
	draw_mesh_textured(meshes_lava,texLava, specLava, 0);

	glUniform2f(sp->u("textureRoll"), 0,0);
	glm::mat4 Mvolcano = glm::scale(M, glm::vec3(2)); // Adjust position if needed
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mvolcano));
	draw_mesh_textured(meshes_vulkan, texWulkan, specWulkan, 0);

	glm::mat4 Mtrex = glm::translate(M, glm::vec3(2.0f, 1, 0.1)); // Adjust position if needed
	Mtrex = glm::rotate(Mtrex, glm::radians(180.0f), glm::vec3(0, 1, 1));
	Mtrex = glm::rotate(Mtrex, glm::radians(90.0f), glm::vec3(0, 1, 0));
	Mtrex = glm::scale(Mtrex, glm::vec3(0.1));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mtrex));
	draw_mesh_textured(meshes_trex, texRex, specRex, 0);

	for (const auto& tree : treepos) {
		glm::mat4 Mtree = glm::translate(M, tree); // Adjust position if needed
		Mtree = glm::scale(Mtree, glm::vec3(0.05f));
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mtree));
		draw_mesh_textured(meshes_tree, texTree, specTree, 0);
	}

	drawParticles(deltaTime, M);

	glfwSwapBuffers(window);
}



int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(1000, 1000, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące

	//Główna pętla
	float angle_x = 0; //Aktualny kąt obrotu obiektu
	float angle_y = 0; //Aktualny kąt obrotu obiektu
	glfwSetTime(0); //Zeruj timer
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		angle_x += speed_x * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		// angle_y += speed_y * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
		angle_y = std::min(std::max(angle_y+speed_y * (float)glfwGetTime(), glm::radians(-30.0f)), glm::radians(2.5f));

		std::chrono::steady_clock::time_point currentTime =
			std::chrono::steady_clock::now();
		std::chrono::duration<float> deltaTimeDuration =
			currentTime - lastUpdateTime;
		double deltaTime = deltaTimeDuration.count(); // Convert duration to seconds
		lastUpdateTime = currentTime;

		glfwSetTime(0); //Zeruj timer
		drawScene(window, angle_x, angle_y, deltaTime); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
