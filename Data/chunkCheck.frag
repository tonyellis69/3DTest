#version 330

#include noise.lib



in vec3 v_texCoord3D; 
layout(location = 0)  out uvec4 outputColor;

uniform vec3 worldPos; 
uniform int slice;
uniform int noCubes;



void main( void ) {
	vec3 noisePos = v_texCoord3D * (noCubes+1) ; //scale texture coordinate up to the dimensions of one chunk 
	noisePos += worldPos; //offset noise position by this chunk's position in (sample) space. 
	
 
	float height = octave_noise_2d(6,0.2,0.02,noisePos.x,noisePos.z);

	height = (height * 0.5) + 0.5;  //convert height to 0 - 1.

	float currVertPos =  32 + noisePos.y;// +16 ; //current vertical position in chunk space 
	//higher numbers lower the cage

	currVertPos = currVertPos  /32; //convert  to a range 1-16
	

	
	float value = currVertPos - height;
	
	//value = clamp(value,0,1);

	if (abs(value) < 1)
		outputColor = uvec4(1,0,0,1);
	else
		outputColor = uvec4(0,0,0,1); 
		
		/*
		value = 1.2 - value;
		value = max(value,0);
		value = sign(value);
		
		outputColor = uvec4(value,0,0,1); 
		*/
}
