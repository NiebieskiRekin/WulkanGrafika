#version 330

uniform sampler2D textureMap0; 
uniform sampler2D textureMap1;
uniform sampler2D textureMap2;
uniform float random;

out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela

in vec2 iTexCoord0;
in vec2 iTexCoord1;
in vec3 n;
in vec3 l1;
in vec3 l2;
in vec3 v;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main(void) {

    //Znormalizowane interpolowane wektory
    vec3 ml1 = normalize(l1);
    vec3 ml2 = normalize(l2);
    vec3 mn = normalize(n);
    vec3 mv = normalize(v);
    //Wektory odbite
    vec3 mr1 = reflect(-ml1, mn);
    vec3 mr2 = reflect(-ml2, mn);

    //Parametry powierzchni
    vec4 kd = mix(texture(textureMap0, iTexCoord0), mix(texture(textureMap1,iTexCoord1), texture(textureMap2, iTexCoord0),0.8), 0.2);
    // vec4 ks = texture(textureMap2, iTexCoord0);
    vec3 ks = vec3(0,0,0);

    //Obliczenie modelu oświetlenia dla obu świateł
    float nl1 = clamp(dot(mn, ml1), 0.0, 1.0);
    float nl2 = clamp(dot(mn, ml2), 0.0, 1.0);
    float rv1 = pow(clamp(dot(mr1, mv), 0.0, 1.0), 50.0);
    float rv2 = pow(clamp(dot(mr2, mv), 0.0, 1.0), 50.0);

    vec3 orange_shift = vec3(kd.r * nl1*random * 2, kd.g*nl1*random, kd.b*0.0);
    vec3 diffuse = orange_shift + kd.rgb*nl2;
 
    // vec3 specular = ks.rgb * (rv1*random/2 + rv2);
    vec3 specular = ks * (rv1*random/2 + rv2);

    // boost saturation
    vec3 out_color = 2*diffuse ;
    out_color = rgb2hsv(out_color);
    out_color = hsv2rgb(vec3(out_color.r,out_color.g*1.3,out_color.b)); // 1.3x saturation
    
    pixelColor = vec4(out_color, kd.a) + vec4(0.4 * specular, 0.0);
    // pixelColor = vec4(diffuse, kd.a);
}
