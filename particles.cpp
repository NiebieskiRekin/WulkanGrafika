#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <vector>
#include <algorithm>

#include "shaderprogram.cpp"

std::shared_ptr<ShaderProgram> sp_particles;


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

const std::vector<glm::float32> particle_box = {
    -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f,
};

const size_t PARTICLES_SIZE = 1000;

std::vector<Particle> particles = std::vector<Particle>(PARTICLES_SIZE);
size_t lastUsedParticle = 0;

GLuint billboard_vertex_buffer;
GLuint particles_position_buffer;
GLuint particles_color_buffer;

std::vector<glm::vec3> particles_position_data = std::vector<glm::vec3>(PARTICLES_SIZE); 
std::vector<glm::vec4> particles_color_data = std::vector<glm::vec4>(PARTICLES_SIZE); 
std::vector<glm::float32> particles_size_data = std::vector<glm::float32>(PARTICLES_SIZE); 



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
void drawParticles(double deltaTime) {
  sp_particles->use();


  updateParticles(deltaTime);  

	std::sort(particles.begin(), particles.end());

    

}
