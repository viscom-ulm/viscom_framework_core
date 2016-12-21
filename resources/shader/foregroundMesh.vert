#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewProjectionMatrix;

out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoords;

void main()
{
    vec4 posV4 = modelMatrix * vec4(position, 1);
    vPosition = vec3(posV4);
    vNormal = normalize(normalMatrix * normal);
    vTexCoords = texCoords;

    gl_Position = viewProjectionMatrix * posV4;
}

