#version 330

layout(location = 0) in vec3 position;


uniform float cubeSize;
uniform vec4 inColour;
uniform vec3 samplePos; //position of bottom nwcorner of this chunk in sample space.


out VertexData {

	vec3 vert;
	float sample;
	
	vec3 opVert;
	float opSample;
} outData;

out vec4 vColour;

#include noise.lib

float getSample(vec3 cornerOffset) {
	vec3 sampleCorner = vec3(samplePos) + position + cornerOffset ;
	sampleCorner.y += 16;
	
	//1. get the surface height at the 2D position of this corner.
	float surfaceHeight = octave_noise_2d(6,0.2,0.02,sampleCorner.x,sampleCorner.z);
	surfaceHeight = (surfaceHeight * 0.5) + 0.5;  //convert to 0 - 1.
	

		
	//2. scale current position down to noise space.
	sampleCorner.y = sampleCorner.y /32;
	
	//3. clip the surface height against the height of this corner. Values outside 1 mean the surface doesn't intersect this point.
	float sampleVal = sampleCorner.y - surfaceHeight;
	
	//4. we now have a sample value for this corner.
//	sampleVal = clamp(sampleVal,0,1);
	return sampleVal;
}



void main() {
	
	vColour = inColour;
	
	outData.vert = position * vec3(cubeSize,cubeSize,cubeSize);
	outData.opVert = outData.vert + vec3(0,0,cubeSize);
	
	outData.sample = getSample(vec3(0,0,0));
	outData.opSample = getSample(vec3(0,0,1));
	
	
	/*
	outData.corner[0] =  ((position + vec4(-0.5,-0.5,-0.5,0)) * cubeScale) ;
	outData.corner[1] =  ((position + vec4(0.5,-0.5,-0.5,0)) * cubeScale) ;
	outData.corner[5] =  ((position + vec4(0.5,-0.5,0.5,0)) * cubeScale) ;
	outData.corner[4] =  ((position + vec4(-0.5,-0.5,0.5,0)) * cubeScale) ;
	
	
	outData.corner[3] =  ((position + vec4(-0.5,0.5,-0.5,0)) * cubeScale) ;
	outData.corner[2] =  ((position + vec4(0.5,0.5,-0.5,0)) * cubeScale) ;
	outData.corner[6] =  ((position + vec4(0.5,0.5,0.5,0)) * cubeScale) ;
	outData.corner[7] = ((position + vec4(-0.5,0.5,0.5,0)) * cubeScale) ;
	
	outData.cornerSample[0] =  getSample(vec4(-0.5,-0.5,-0.5,0));
	outData.cornerSample[1] =  getSample(vec4(0.5,-0.5,-0.5,0));
	outData.cornerSample[5] =  getSample(vec4(0.5,-0.5,0.5,0));
	outData.cornerSample[4] =  getSample(vec4(-0.5,-0.5,0.5,0));
	
	
	outData.cornerSample[3] =  getSample(vec4(-0.5,0.5,-0.5,0));
	outData.cornerSample[2] =  getSample(vec4(0.5,0.5,-0.5,0));
	outData.cornerSample[6] =  getSample(vec4(0.5,0.5,0.5,0));
	outData.cornerSample[7] =  getSample(vec4(-0.5,0.5,0.5,0));
	*/
	
}
