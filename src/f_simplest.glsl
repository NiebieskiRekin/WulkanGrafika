#version 330

uniform sampler2D textureMap0; 
uniform sampler2D textureMap1;

out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela

in vec2 iTexCoord0;
in vec2 iTexCoord1;
in vec3 n;
in vec3 l1;
in vec3 l2;
in vec3 v;

void main(void) {

    //Znormalizowane interpolowane wektory
    vec3 ml1 = normalize(l1);
    vec3 ml2 = normalize(l2);
    vec3 mn = normalize(n);
    vec3 mv = normalize(v);
    //Wektory odbite
    // vec3 mr1 = reflect(-ml1, mn);
    // vec3 mr2 = reflect(-ml2, mn);

    //Parametry powierzchni
    // vec4 kd = mix(texture(textureMap0, iTexCoord0), texture(textureMap1, iTexCoord1), 0.2);
    vec4 kd = texture(textureMap0, iTexCoord0);
    // vec3 ks = vec3(1, 1, 1);

    //Obliczenie modelu oświetlenia dla obu świateł
    float nl1 = clamp(dot(mn, ml1), 0.0, 1.0);
    float nl2 = clamp(dot(mn, ml2), 0.0, 1.0);
    // float rv1 = pow(clamp(dot(mr1, mv), 0.0, 1.0), 50.0);
    // float rv2 = pow(clamp(dot(mr2, mv), 0.0, 1.0), 50.0);

    vec3 diffuse = kd.rgb * (nl1 + nl2);
    // vec3 specular = ks * (rv1 + rv2);

    // pixelColor = vec4(diffuse, kd.a) + vec4(0.4 * specular, 0.0);
    pixelColor = vec4(diffuse, kd.a);
}
