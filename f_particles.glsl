#version 330 core

in vec2 UV;
in vec4 color_particle;

uniform sampler2D particle_tex;

out vec4 color;

void main() {
    color = vec4(1,1,0,1); 
    // texture(particle_tex, UV) * color_particle;    
}
