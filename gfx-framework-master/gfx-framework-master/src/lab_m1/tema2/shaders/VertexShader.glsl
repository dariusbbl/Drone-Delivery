#version 330 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Output the modified position to the Fragment Shader
out vec3 fragPosition;

float random (in vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

void main() {
	// Set the amplitude and scale of the noise
    float u_ampl = 0.11f;
    float u_noiseScale = 5.0f;

    vec3 modifiedPosition = v_position;

    // Scale the xz coordinates for the noise function
    vec2 noiseInput = v_position.xz * u_noiseScale;

    // Adjust height based on the noise
    modifiedPosition.y = noise(noiseInput) * u_ampl;

    fragPosition = vec3(Model * vec4(modifiedPosition, 1.0));

    gl_Position = Projection * View * vec4(fragPosition, 1.0);


}
