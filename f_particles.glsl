#version 330 core

in vec2 UV;
in vec4 color_particle;

uniform sampler2D particle_tex;

out vec4 color;

void main() {
    color = texture(particle_tex, UV) * color_particle;    
}
