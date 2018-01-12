#version 330 core

out vec2 texCoord;

const vec2 pos_data[3] = vec2[]
(
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

const vec2 tex_data[3] = vec2[]
(
    vec2(0.0, 0.0),
    vec2(2.0, 0.0),
    vec2(0.0, 2.0)
);

void main()
{
    texCoord = tex_data[ gl_VertexID ];
    gl_Position = vec4( pos_data[ gl_VertexID ], 0.0, 1.0 );
}
