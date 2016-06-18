#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 inColour;
layout(location = 2) in vec4 normal;

uniform mat4 mvpMatrix; //model to view (camera) to perspective matrix

out vec4 outColour;
void main()
{
	gl_Position = mvpMatrix * position;
	outColour = inColour;
	
}
