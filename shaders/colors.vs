#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	FragPos = vec3(model * vec4(aPos, 1.0)); // pass normal to fragment shader vertex multiplied by model matrix to get world space normal
	//we need to transform the normal vector by the inverse transpose of the model matrix to ensure that it is correctly transformed in world space, especially when the model matrix includes non-uniform scaling.
	//this is costly in terms of performance, but it's necessary to ensure correct lighting calculations in the fragment shader, especially when dealing with non-uniform scaling in the model matrix.
	Normal = mat3(transpose(inverse(model))) * aNormal;  
}