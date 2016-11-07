#version 330 core
in vec3 v_TexCoord;

out vec4 color;

// Texture samplers
uniform sampler2D tex;

void main()
{
	vec2 coord = vec2(v_TexCoord.s / v_TexCoord.p, v_TexCoord.t / v_TexCoord.p);

	color = texture(tex, coord);
}