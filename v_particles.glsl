#version 330 core

layout(location = 0) in vec3 particlePosition;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform float particleSize;

void main() {
    gl_Position = P * V * M * vec4(particlePosition, 1.0);
    gl_PointSize = particleSize;
}
