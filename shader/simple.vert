void main()
{
    gl_FrontColor = gl_Color; //Added line
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position =  gl_ModelViewProjectionMatrix * gl_Vertex;
}
