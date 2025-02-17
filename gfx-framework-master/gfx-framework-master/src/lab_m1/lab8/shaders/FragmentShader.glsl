#version 330

// Input
in vec3 world_position;
in vec3 world_normal;

// Uniforms for light properties
uniform vec3 light_direction;
uniform vec3 light_position;
uniform bool is_spotlight;
uniform float cutoff_angle;

uniform vec3 light_direction2;
uniform vec3 light_position2;
uniform bool is_spotlight2;
uniform float cutoff_angle2;


uniform vec3 eye_position;
uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;



uniform vec3 object_color;

// Output
layout(location = 0) out vec4 out_color;


void main()
{
    vec3 N = normalize(world_normal);
    vec3 V = normalize(eye_position - world_position);

    vec3 L = normalize(light_position - world_position);
    vec3 R = reflect(-L, N);
    
    vec3 L2 = normalize(light_position2 - world_position);
    vec3 R2 = reflect(-L2, N);
    
    // TODO(student): Define ambient, diffuse and specular light components
    float ambient_light = 0.25;
    float diffuse_light = max(dot(N, L), 0.0);
    float specular_light = 0;

    float diffuse_light2 = max(dot(N, L2), 0.0);
    float specular_light2 = 0;

    vec3 ambient = ambient_light * object_color;
    vec3 diffuse = material_kd * diffuse_light * object_color;

    vec3 diffuse2 = material_kd * diffuse_light2 * object_color;
    
    if (diffuse_light > 0)
    {
        specular_light = pow(max(dot(R, V), 0.0), material_shininess);
    }
    vec3 specular = material_ks * specular_light * vec3(0.9804, 0.0353, 0.0353);

    if (diffuse_light2 > 0)
    {
        specular_light2 = pow(max(dot(R2, V), 0.0), material_shininess);
    }
    vec3 specular2 = material_ks * specular_light2 * vec3(0.1255, 0.0314, 0.9686);
    
    if (is_spotlight)
    {
        float cutoff = cos(radians(cutoff_angle));
        vec3 spot_direction = normalize(light_direction);
        float spot_light = dot(-L, spot_direction);

        if (spot_light > cutoff)
        {
            float attenuation = (spot_light - cutoff) / (1.0 - cutoff);
            diffuse *= attenuation;
            specular *= attenuation;
        }
        else
        {
            diffuse = vec3(0.0, 0.0, 0.0);
            specular = vec3(0.0, 0.0, 0.0);
        }
    }

    if (is_spotlight2)
    {
        float cutoff2 = cos(radians(cutoff_angle2));
        vec3 spot_direction2 = normalize(-light_direction2); 
        float spot_light2 = dot(L2, spot_direction2); 

        

        if (spot_light2 > cutoff2)
        {
            float attenuation2 = (spot_light2 - cutoff2) / (1.0 - cutoff2);
            diffuse2 *= attenuation2;
            specular2 *= attenuation2;
        }
        else
        {
            diffuse2 = vec3(0.0);
            specular2 = vec3(0.0);
        }
    }

    vec3 total_light = ambient + diffuse + specular + diffuse2 + specular2;

    // TODO(student): Write pixel out color
    out_color = vec4(total_light, 1.0);

}
