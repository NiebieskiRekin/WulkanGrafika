
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
#include "myCube.h"
#include "myTeapot.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

	std::chrono::steady_clock::time_point lastUpdateTime = std::chrono::steady_clock::now();

	ShaderProgram* sp;
	ShaderProgram* sp_particles;

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

	struct Particle {
		glm::vec3 position;
		glm::vec3 velocity;
		glm::vec4 color;
		float size;
		float lifespan; // Remaining lifespan of the particle
	};

	std::vector<Particle> particles;
	GLuint particleVAO, particleVBO;
	std::vector<glm::vec3> particlePositions;

	void generateParticle() {
		// Generate a new particle with random attributes
		Particle p;
		p.position = glm::vec3(0,0,0);
		p.velocity = glm::vec3(0,-3,0);
		//p.color = glm::vec4(1,1,1,1);
		p.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		p.size = 200000000;
		p.lifespan = 100;
		particles.push_back(p);
	}

	// Function to update particles
	void updateParticles(float deltaTime) {
		particlePositions.clear();
		for (auto& particle : particles) {
			particle.position += particle.velocity * deltaTime;
			float alpha = 1; // particle.lifespan / 100000000.0f;
			particle.color.a = alpha;
			particle.lifespan -= deltaTime;
			particlePositions.push_back(particle.position);
			// Print particle positions
			std::cout << std::fixed
				<< "Particle Position: x=" << particle.position.x
				<< ", y=" << particle.position.y
				<< ", z=" << particle.position.z << std::endl;
		}
		particles.erase(std::remove_if(particles.begin(), particles.end(),
			[](const Particle& p) { return p.lifespan <= 0; }),
			particles.end());
	}

	// Function to render particles
	void drawParticles() {
		sp_particles->use();

		glBindVertexArray(particleVAO);

		// Update the particle VBO with the new particle positions
		glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, particlePositions.size() * sizeof(glm::vec3), particlePositions.data());

		// Set uniforms for each particle and draw
		for (const auto& particle : particles) {
			glUniform4fv(sp_particles->u("particleColor"), 1, glm::value_ptr(particle.color));
			glUniform1f(sp_particles->u("particleSize"), particle.size);
			glDrawArrays(GL_POINTS, 0, particlePositions.size());
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}



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
	float* colors = myTeapotColors;
	//int vertexCount = myTeapotVertexCount;

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

	void loadModel(std::string plik, std::vector<MeshData>& meshContainer)
	{
		using namespace std;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(plik, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
		if (!scene) {
			cerr << "Error loading model: " << importer.GetErrorString() << endl;
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
		sp_particles = new ShaderProgram("v_particles.glsl", NULL, "f_particles.glsl");

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

		glGenVertexArrays(1, &particleVAO);
		glGenBuffers(1, &particleVBO);

		glBindVertexArray(particleVAO);

		glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
		glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0); // Assuming the layout(location = 0) in the vertex shader for position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}


	//Zwolnienie zasobów zajętych przez program
	void freeOpenGLProgram(GLFWwindow* window) {
		//************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************

		delete sp;

		glDeleteTextures(1, &texWulkan);
	}

	void draw_mesh_textured(const std::vector<MeshData>& mesh_vec, GLuint texture, GLint v0) {
		glUniform1i(sp->u("textureMap0"), v0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

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

	//Procedura rysująca zawartość sceny
	void drawScene(GLFWwindow* window, float angle_x, float angle_y) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 V = glm::lookAt(
			glm::vec3(0, 1, -5),
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

		//glUniform1i(sp->u("textureMap1"), 1);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, texNiebo);

		//draw_mesh_textured(meshes_floor, texWulkan, 0);

		//glm::mat4 Mlava = glm::scale(M, glm::vec3(2.01)); // Adjust position if needed
		//glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mlava));
		//draw_mesh_textured(meshes_lava, texLava, 0);

		//glm::mat4 Mvolcano = glm::scale(M, glm::vec3(2)); // Adjust position if needed
		//glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mvolcano));
		//draw_mesh_textured(meshes_vulkan, texWulkan, 0);

		//glm::mat4 Mtrex = glm::translate(M, glm::vec3(2.0f, 1, 0.1)); // Adjust position if needed
		//Mtrex = glm::rotate(Mtrex, glm::radians(180.0f), glm::vec3(0, 1, 1));
		//Mtrex = glm::rotate(Mtrex, glm::radians(90.0f), glm::vec3(0, 1, 0));
		//Mtrex = glm::scale(Mtrex, glm::vec3(0.1));
		//glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mtrex));
		//draw_mesh_textured(meshes_trex, texRex, 0);

		//for (const auto& tree : treepos) {
		//	glm::mat4 Mtree = glm::translate(M, tree); // Adjust position if needed
		//	Mtree = glm::scale(Mtree, glm::vec3(0.05f));
		//	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(Mtree));
		//	draw_mesh_textured(meshes_tree, texTree, 0);
		//}
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
		//glfwSetTime(0); //Zeruj timer
		while (!glfwWindowShouldClose(window)) {
			//angle_x += speed_x * glfwGetTime(); // Update rotation angle
			//angle_y += speed_y * glfwGetTime(); // Update rotation angle
			std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
			std::chrono::duration<float> deltaTimeDuration = currentTime - lastUpdateTime;
			float deltaTime = deltaTimeDuration.count(); // Convert duration to seconds
			lastUpdateTime = currentTime;
			glfwSetTime(0); // Reset timer

			// Generate new particles
			generateParticle();

			// Update particles
			updateParticles(deltaTime);

			// Render scene
			drawScene(window, angle_x, angle_y);

			// Render particles
			drawParticles();

			glfwSwapBuffers(window); // Swap buffers
			glfwPollEvents(); // Process events
		}

		freeOpenGLProgram(window);

		glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
		glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
		exit(EXIT_SUCCESS);
	}