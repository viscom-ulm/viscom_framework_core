#version 330 core

layout(location = 0) in vec3 vertex_coordinates;
//layout(location = 1) in vec2 texture_coordinates_in;

out vec2 texture_coordinates_frag;

void main()
{
	gl_Position = vec4(vertex_coordinates, 1);
	texture_coordinates_frag = vec2(vertex_coordinates) * 0.5 + vec2(0.5, 0.5);
	//texture_coordinates_frag = vec2(vertex_coordinates);
    //texture_coordinates_frag = vec2(texture_coordinates_in.x, 1.0 - texture_coordinates_in.y);
}