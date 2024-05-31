#version 330

//Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

//Atrybuty
in vec4 vertex; //wspolrzedne wierzcholka w przestrzeni modelu
in vec4 normal; //wektor normalny w przestrzeni modelu
in vec2 texCoord0;

//Zmienne interpolowane
out vec3 l1;
out vec3 l2;
out vec3 n;
out vec3 v;
out vec2 iTexCoord0; 
out vec2 iTexCoord1;

void main(void) {
    vec4 lp1 = vec4(0, 3, 0, 1); //pozycja pierwszego światła, przestrzeń świata
    vec4 lp2 = vec4(5, 3, 0, 1); //pozycja drugiego światła, przestrzeń świata

    l1 = normalize((V * lp1 - V * M * vertex).xyz); //wektor do pierwszego światła w przestrzeni oka
    l2 = normalize((V * lp2 - V * M * vertex).xyz); //wektor do drugiego światła w przestrzeni oka
    v = normalize((vec4(0, 0, 0, 1) - V * M * vertex).xyz); //wektor do obserwatora w przestrzeni oka
    n = normalize((V * M * normal).xyz); //wektor normalny w przestrzeni oka
    
    iTexCoord0 = texCoord0;
    iTexCoord1 = (n.xy + 1) / 2;
    
    gl_Position = P * V * M * vertex;
}
