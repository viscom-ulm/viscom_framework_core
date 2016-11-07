#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 texCoord;

//uniform mat4 transform;

out vec3 v_TexCoord;
void main()
{
	//gl_Position = transform * vec4(vertPosition, 1.0);
	gl_Position = vec4(position, 1.0);

	v_TexCoord = texCoord;
}