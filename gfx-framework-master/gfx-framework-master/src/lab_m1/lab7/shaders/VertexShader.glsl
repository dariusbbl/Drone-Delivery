#version 330

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Uniforms for light properties
uniform vec3 light_position;
uniform vec3 eye_position;
uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;
uniform vec3 object_color;

// Output value to fragment shader
out vec3 color;

void main()
{
    // Compute world space vectors
    vec3 world_position = vec3(Model * vec4(v_position, 1.0));
    vec3 N = normalize(mat3(Model) * v_normal);
    vec3 L = normalize(light_position - world_position);
    vec3 V = normalize(eye_position - world_position);
    vec3 H = normalize(L + V);

    // Define ambient light component
    float ambient_light = 0.25;
    // vec3 ambient_color = ambient_light * object_color;

    // Compute diffuse light component
    float diffuse_light = max(dot(N, L), 0.0) * material_kd;
    // vec3 diffuse_color = diffuse_light * object_color * material_kd;

    // Compute specular light component
    vec3 R = reflect(-L, N);
    float specular_light = 0;

    if (diffuse_light > 0)
    {
        specular_light = pow(max(dot(V, R), 0.0), material_shininess);
    }

    // Compute light
    float dist = length(light_position - world_position);
    float color_f = ambient_light + (diffuse_light + specular_light) * (1/(1 + dist * dist));

    // Send color light output to fragment shader
    color = color_f * object_color;
    gl_Position = Projection * View * Model * vec4(v_position, 1.0);
}