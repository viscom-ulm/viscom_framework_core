#version 330 core
in vec3 fragColor;
layout (location = 0) out vec4 color;

uniform vec4 polygonColor;

void main()
{	
	if(polygonColor.x >= 0.0) {
		color = polygonColor; //stencil test with gamma correction
	} else {
		color = vec4(fragColor, 1.0); //overlap transistion
	}
	
	/*if(polygonColor.a < 1.0 && polygonColor.a > 0.0) {
		color = vec4(fragColor, 1.0);
	} else {
		color = polygonColor;
	} */   
}