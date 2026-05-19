#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 uLightDir;
uniform vec3 uColor;
uniform vec3 uAmbient;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    float diff = max(dot(N, L), 0.0);
    vec3 ambient = uAmbient * uColor;
    vec3 diffuse = diff * uColor;
    FragColor = vec4(ambient + diffuse, 1.0);
}