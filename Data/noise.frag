#version 330

#include noise.lib


in vec3 v_texCoord3D;
layout(location = 0)  out vec4 outputColor;

uniform int vertsPerChunkEdge;
uniform vec3 worldPos;
uniform int slice;
uniform int noCubes;
//TO DO: have a baseline that worldPos.y can be compared to, simplifying the currentVertPos calculation


void main( void ) {
	
	vec3 noisePos = v_texCoord3D * (noCubes+1) ; //scale texture coordinate up to the dimensions of one chunk 
	noisePos += worldPos; //offset noise position by this chunk's position in world space. 
 
  
	float height = octave_noise_2d(6,0.2,0.02,noisePos.x,noisePos.z);	//height = height * (254) / 2 + (254) / 2; //scale up to 0-254;
	height = (height * 0.5) + 0.5;  //convert height to 0 - 1.
	
	//new stuff
	float currVertPos =  worldPos.y + slice +16; //current vertical position in chunk space
	currVertPos /= 32;
//	height *= 34;
		
	float value = ( currVertPos - height); 
	
	//value now from -24 to 24+
	
	//value = (value + 25) /250; // should be around 1 now
	
	//value = value /33;
//	value = clamp(value,-2.5,2.5);
	//value = max(value,-1);
	value = clamp(value,0,1);
	
	outputColor = vec4(value,0,0,1);
	
}
