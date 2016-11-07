#version 120

uniform sampler2D tex;

void main()
{
	vec2 coord = vec2(gl_TexCoord[0].s / gl_TexCoord[0].p, gl_TexCoord[0].t / gl_TexCoord[0].p);
	//vec2 coord = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t);
	vec4 texel = vec4(texture2D(tex, coord).rgb, 1);
    if((gl_Color.r != 1) || (gl_Color.g != 1) || (gl_Color.b != 1)) {
        gl_FragColor = gl_Color;
    } else {
        gl_FragColor.rgb = texel.rgb;
    }
	gl_FragColor.a = 1.0;
}
