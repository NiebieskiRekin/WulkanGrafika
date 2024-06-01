#version 330 core

layout(location = 0) in vec3 square;
layout(location = 1) in vec3 position; // center of the particle
layout(location = 2) in float size;
layout(location = 3) in vec4 color;

out vec2 UV;
out vec4 color_particle;


uniform vec3 camera_right;
uniform vec3 camera_up;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main() {

    vec3 v_pos = position + camera_right * square.x * size + camera_up * square.y * size;
    
    gl_Position = P * V * vec4(particlePosition, 1.0f);

    UV = vec2(0.5) + square.xy;

    color_particle = color;
}
