#version 420

 layout(points) in;
 layout(triangle_strip, max_vertices = 12) out;

in VertexData
{
    vec4 corner[8];
	float cornerSample[8];
} vert[];

in vec4 vColour[];

out vec4 gl_Position;
out	vec4 outColour;
out	vec4 normal;

 void main()
{   	
		outColour = vColour[0]; 
	
		float iVertTest = 0.5;
		int iFlagIndex = 0;
		/*for(int iVertexTest = 0; iVertexTest < 8; iVertexTest++){
			if(vert[0].cornerSample[iVertexTest] <= iVertexTest) 
				iFlagIndex++;//|= 1<<iVertexTest;
		}*/
		
		
		
	//Determine the index into the edge table which 
	//tells us which vertices are inside of the surface 
	iFlagIndex = int(vert[0].cornerSample[0] < iVertTest); 
	iFlagIndex += int(vert[0].cornerSample[1] < iVertTest)*2; 
	iFlagIndex += int(vert[0].cornerSample[2] < iVertTest)*4; 
	iFlagIndex += int(vert[0].cornerSample[3] < iVertTest)*8; 
	iFlagIndex += int(vert[0].cornerSample[4] < iVertTest)*16; 
	iFlagIndex += int(vert[0].cornerSample[5] < iVertTest)*32; 
	iFlagIndex += int(vert[0].cornerSample[6] < iVertTest)*64; 
	iFlagIndex += int(vert[0].cornerSample[7] < iVertTest)*128; 
	 
	 
	//Cube is entirely in/out of the surface 
	if (iFlagIndex ==0 || iFlagIndex == 255) 
		return; 
		
	
		
		
		normal = vec4(normalize(cross(vec3(vert[0].corner[3] - vert[0].corner[2] ),
								vec3(vert[0].corner[3] - vert[0].corner[7] ))),0);
		
		 gl_Position =  vert[0].corner[3];
		 EmitVertex();
		 
		  gl_Position =  vert[0].corner[2];
		 EmitVertex();
		  gl_Position =  vert[0].corner[7];
		 EmitVertex();
		 EndPrimitive();
		 
		 normal = vec4(normalize(cross(vec3(vert[0].corner[6] - vert[0].corner[7] ),
								vec3(vert[0].corner[6] - vert[0].corner[2] ))),0);
		  gl_Position =  vert[0].corner[6];
		 EmitVertex();
		  gl_Position =  vert[0].corner[7];
		 EmitVertex();
		 gl_Position =  vert[0].corner[2];
		 EmitVertex();
        EndPrimitive();
		
		
		normal = vec4(normalize(cross(vec3(vert[0].corner[7] - vert[0].corner[6] ),
								vec3(vert[0].corner[7] - vert[0].corner[4] ))),0);
		 gl_Position =  vert[0].corner[7];
		 EmitVertex();
		  gl_Position =  vert[0].corner[6];
		 EmitVertex();
		  gl_Position =  vert[0].corner[4];
		 EmitVertex();
		 EndPrimitive();
		 
		 normal = vec4(normalize(cross(vec3(vert[0].corner[5] - vert[0].corner[4] ),
								vec3(vert[0].corner[5] - vert[0].corner[6] ))),0);
		  gl_Position =  vert[0].corner[5];
		 EmitVertex();
		  gl_Position =  vert[0].corner[4];
		 EmitVertex();
		 gl_Position =  vert[0].corner[6];
		 EmitVertex();
        EndPrimitive();
 }