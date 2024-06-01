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

#include "camera.cpp"
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
std::vector<glm::float32> particles_size_data = std::vector<glm::float32>(PARTICLES_SIZE); 

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
	glBufferData(GL_ARRAY_BUFFER, PARTICLES_SIZE * 4 * sizeof(glm::float32), NULL, GL_STREAM_DRAW);
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
  sp_particles->use();


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


	// glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Use our shader
	// glUseProgram(programID);

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
