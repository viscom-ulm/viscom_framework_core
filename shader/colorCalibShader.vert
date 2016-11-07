#version 330 core
 
layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertAlpha;

out vec3 fragColor;

uniform mat4 transform;

void main()
{
	gl_Position = transform * vec4(vertPosition, 1.0);
	//gl_Position = vec4(vertPosition, 1.0); //TODO: test 15.06.
	//pow((1.f/x),(1.f/2.2f)
	/*
	//TODO: test auskommentiert am 16.06.
	if(vertAlpha.x == 0.5) {
		float val = pow(0.5, 1.0/2.2);
		fragColor = vec3(val);
	} else {
		fragColor = vertAlpha;
	}
	*/	
	fragColor = vertAlpha;
}