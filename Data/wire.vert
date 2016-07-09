#version 330

layout(location = 0) in vec3 position;

uniform mat4 cameraToClipMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 mvpMatrix; //model to view (camera) to perspective matrix
uniform vec3 scale;


void main()
{
	vec4 pos = vec4(position,1);
	pos.xyz *= scale;
	gl_Position = cameraToClipMatrix * modelToWorldMatrix * pos ;
	gl_Position = mvpMatrix * pos ;
	
}
