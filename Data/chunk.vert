#version 420

layout(location = 0) in vec4 position;


uniform vec4 cubeScale;
uniform vec4 inColour;
uniform vec3 samplePos; //position of bottom nwcorner of this chunk in sample space.
uniform float hFactor; //Vertical adjustment.

out VertexData {
    vec4 corner[8];
	float cornerSample[8];
} outData;

out vec4 vColour;

#include noise.lib

float getSample(vec4 cornerOffset) {
	vec4 sampleCorner = vec4(samplePos,1) + position + cornerOffset ;
	sampleCorner.y += 16;
	
	//1. get the surface height at the 2D position of this corner.
	float surfaceHeight = octave_noise_2d(6,0.2,0.02,sampleCorner.x,sampleCorner.z);
	surfaceHeight = (surfaceHeight * 0.5) + 0.5;  //convert to 0 - 1.
	

		
	//2. scale current position down to noise space.
	sampleCorner.y = sampleCorner.y /32;
	
	//3. clip the surface height against the height of this corner. Values outside 1 mean the surface doesn't intersect this point.
	float sampleVal = sampleCorner.y - surfaceHeight;
	
	//4. we now have a sample value for this corner.
	sampleVal = clamp(sampleVal,0,1);
	return sampleVal;
}



void main()
{
	//gl_Position = mvpMatrix  * (position * cubeScale) ;
	vColour = inColour;
	
	outData.corner[0] =  ((position + vec4(-0.3,-0.3,-0.3,0)) * cubeScale) ;
	outData.corner[1] =  ((position + vec4(0.3,-0.3,-0.3,0)) * cubeScale) ;
	outData.corner[2] =  ((position + vec4(0.3,-0.3,0.3,0)) * cubeScale) ;
	outData.corner[3] =  ((position + vec4(-0.3,-0.3,0.3,0)) * cubeScale) ;
	
	
	outData.corner[4] =  ((position + vec4(-0.3,0.3,-0.3,0)) * cubeScale) ;
	outData.corner[5] =  ((position + vec4(0.3,0.3,-0.3,0)) * cubeScale) ;
	outData.corner[6] =  ((position + vec4(0.3,0.3,0.3,0)) * cubeScale) ;
	outData.corner[7] = ((position + vec4(-0.3,0.3,0.3,0)) * cubeScale) ;
	
	outData.cornerSample[0] =  getSample(vec4(-0.5,-0.5,-0.5,0));
	outData.cornerSample[1] =  getSample(vec4(0.5,-0.5,-0.5,0));
	outData.cornerSample[2] =  getSample(vec4(0.5,-0.5,0.5,0));
	outData.cornerSample[3] =  getSample(vec4(-0.5,-0.5,0.5,0));
	
	
	outData.cornerSample[4] =  getSample(vec4(-0.5,0.5,-0.5,0));
	outData.cornerSample[5] =  getSample(vec4(0.5,0.5,-0.5,0));
	outData.cornerSample[6] =  getSample(vec4(0.5,0.5,0.5,0));
	outData.cornerSample[7] =  getSample(vec4(-0.5,0.5,0.5,0));
	
	
}
