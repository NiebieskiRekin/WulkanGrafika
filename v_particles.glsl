#version 330 core

layout(location = 0) in vec3 square;
layout(location = 1) in vec3 position; // center of the particle
layout(location = 2) in float size;
layout(location = 3) in vec4 color;

out vec2 UV;
out vec4 color_particle;


uniform vec3 camera_right;
uniform vec3 camera_up;
uniform mat4 PV;

void main() {

    vec3 v_pos = position + camera_right * square.x * size + camera_up * square.y * size;
    
    gl_Position = PV * vec4(position, 1.0f);

    UV = vec2(0.5) + square.xy;

    color_particle = color;
}
