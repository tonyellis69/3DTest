#version 330


//A vertex somewhere on the outer layer of a chunk
layout(location = 0) in vec3 vertPos;

//The position in sample space of the bottom nw corner of the chunk we're checking 
uniform vec3 nwSamplePos;
uniform float LoDscale;

out VertexData {
	float vertSample; //The sample value at this vertex, passed on to the geometry shader.
} outData;


#include noise.lib

void main () {
	//find the position in sample space of this vertex
	vec3 vertSamplePos = nwSamplePos + (vertPos * LoDscale);

		
	//get a height value at this point
	float surfaceHeight = octave_noise_2d(6,0.2,0.02,vertSamplePos.x,vertSamplePos.z);
	surfaceHeight = (surfaceHeight * 0.5) + 0.5;  //convert to 0 - 1.

	//scale down our vertical position in sample space to be 16:1 proportional to noise space.
	vertSamplePos.y  = vertSamplePos.y / 32;// 16;  //32;

	//clip the surface height against our sampling height
	float vertSample = vertSamplePos.y - surfaceHeight;

//	vertSample = abs(vertSample);
//	vertSample = clamp(vertSample,0,1);
	outData.vertSample = vertSample; // output the result to the geometry shader

}