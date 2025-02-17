#version 330

// Input
in vec2 texcoord;

// Uniform properties
uniform sampler2D texture_1;
uniform sampler2D texture_2;
// TODO(student): Declare various other uniforms
uniform bool mix_texture; // daca se face mix sau nu
// Output
layout(location = 0) out vec4 out_color;


void main()
{
    // TODO(student): Calculate the out_color using the texture2D() function.
    //out_color = vec4(1);

    vec4 color_1 = texture(texture_1, texcoord);
    vec4 color_2 = texture(texture_2, texcoord);

    // daca se face mix, se face un mix intre cele 2 texturi
    if (mix_texture == true) {
		out_color = mix(color_1, color_2, 0.5f);
	} else {
		out_color = color_1; // se afiseaza doar prima textura
    }

    if (out_color.a < 0.5f) { // discard pentru pixeli cu transparenta mai mica de 0.5f
		discard;
	}

}
