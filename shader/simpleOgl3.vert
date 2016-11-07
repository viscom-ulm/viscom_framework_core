#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 texCoord;

out vec3 v_TexCoord;

void main()
{
	gl_Position = vec4(position, 1.0f);
	// We swap the y-axis by substracing our coordinates from 1.
	//v_TexCoord = vec3(texCoord.s, 1.0 - texCoord.t, texCoord.p);
	v_TexCoord = texCoord;
}