#version 330 core
in vec3 v_TexCoord;
in vec4 gl_FragCoord;

out vec4 color;

// Texture samplers
uniform sampler2D tex;
uniform sampler2D alphaTex;

void main()
{
    const float gamma = 1.0/2.2;

    vec2 coord = vec2(v_TexCoord.s / v_TexCoord.p, v_TexCoord.t / v_TexCoord.p);
    vec2 screenSize = vec2(textureSize(alphaTex, 0));
    vec2 screenCoords = gl_FragCoord.xy / screenSize;

    vec4 colorTexture = texture(tex, coord);
    vec4 alpha = texture(alphaTex, screenCoords);
    
    alpha = vec4(vec3(pow(alpha.r, gamma)), 1.0f);

    color = vec4(colorTexture.rgb * alpha.rgb, 1.0f);
}

