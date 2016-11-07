#version 330 core

//already switched y coordinates for the texture_coordinates;
in vec2 texture_coordinates_frag;

out vec4 color;

uniform sampler2D tex;

void main()
{
    //color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    //color = texture(tex, texture_coordinates_frag);
	color = texture(tex, texture_coordinates_frag);
}