#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 texCoord;

out vec3 v_TexCoord;

void main()
{
    gl_Position = vec4(position, 1.0f);
    v_TexCoord = texCoord;
}