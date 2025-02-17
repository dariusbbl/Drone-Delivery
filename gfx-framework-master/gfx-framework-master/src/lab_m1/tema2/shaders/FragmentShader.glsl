#version 330 core

in vec3 fragPosition;

out vec4 color;

void main() {
    vec3 color1 = vec3(0.3f, 0.8f, 0.3f);
    vec3 color2 = vec3(0.0f, 0.3f, 0.0f);
    vec3 terrainColor = mix(color1, color2, smoothstep(0.0, 0.5, fragPosition.y));
    color = vec4(terrainColor, 1.0);
}
