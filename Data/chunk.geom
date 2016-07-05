#version 330



#extension GL_EXT_geometry_shader4 : enable 
#extension GL_EXT_gpu_shader4 : enable 
 

 layout(lines_adjacency) in;
 layout(triangle_strip, max_vertices = 16) out;

in VertexData
{
 //   vec4 corner[8];
//	float cornerSample[8];
	
	vec4 vert;
	float sample;
	
	vec4 opVert;
	float opSample;
} vert[];

in vec4 vColour[];

out vec4 gl_Position;
out	vec4 outColour;
out	vec4 normal;

//uniform isampler2D edgeTableTex; 
//Triangles table texture 
uniform isampler2D triTableTex; 

uniform float iVertTest = 0.5; 

//Compute interpolated vertex along an edge 
vec4 vertexInterp(float isolevel, vec4 v0, float l0, vec4 v1, float l1){ 
	return mix(v0, v1, (isolevel-l0)/(l1-l0)); 
}

//Get triangle table value 
int triTableValue(int i, int j){ 
	//return texelFetch2D(triTableTex, ivec2(j, i), 0).a; 
	return texelFetch(triTableTex, ivec2(j,i),0).r;
} 



 void main()
{   	
		outColour = vColour[0]; 
	

		int iFlagIndex = 0;

			
		
	//Determine the index into the edge table which 
	//tells us which vertices are inside of the surface 
	iFlagIndex = int(vert[0].sample < iVertTest); 
	iFlagIndex += int(vert[1].sample < iVertTest)*2; 
	iFlagIndex += int(vert[2].sample < iVertTest)*4; 
	iFlagIndex += int(vert[3].sample < iVertTest)*8; 
	iFlagIndex += int(vert[0].opSample < iVertTest)*16; 
	iFlagIndex += int(vert[1].opSample < iVertTest)*32; 
	iFlagIndex += int(vert[2].opSample < iVertTest)*64; 
	iFlagIndex += int(vert[3].opSample < iVertTest)*128; 
	 
	 
	//Cube is entirely in/out of the surface 
	if (iFlagIndex ==0 || iFlagIndex == 255) 
		return; 
		
		
		
		
		
		
	vec4 vertlist[12]; 
	 
	//Find the vertices where the surface intersects the cube 
	vertlist[0] = vertexInterp(iVertTest, vert[0].vert, vert[0].sample, vert[1].vert, vert[1].sample); 
	vertlist[1] = vertexInterp(iVertTest, vert[1].vert, vert[1].sample, vert[2].vert, vert[2].sample); 
	vertlist[2] = vertexInterp(iVertTest, vert[2].vert, vert[2].sample, vert[3].vert, vert[3].sample); 
	vertlist[3] = vertexInterp(iVertTest, vert[3].vert, vert[3].sample, vert[0].vert, vert[0].sample); 
	vertlist[4] = vertexInterp(iVertTest, vert[0].opVert, vert[0].opSample, vert[1].opVert, vert[1].opSample); 
	vertlist[5] = vertexInterp(iVertTest, vert[1].opVert, vert[1].opSample, vert[2].opVert, vert[2].opSample); 
	vertlist[6] = vertexInterp(iVertTest, vert[2].opVert, vert[2].opSample, vert[3].opVert, vert[3].opSample); 
	vertlist[7] = vertexInterp(iVertTest, vert[3].opVert, vert[3].opSample, vert[0].opVert, vert[0].opSample); 
	vertlist[8] = vertexInterp(iVertTest, vert[0].vert, vert[0].sample, vert[0].opVert, vert[0].opSample); 
	vertlist[9] = vertexInterp(iVertTest, vert[1].vert, vert[1].sample, vert[1].opVert, vert[1].opSample); 
	vertlist[10] = vertexInterp(iVertTest, vert[2].vert, vert[2].sample, vert[2].opVert, vert[2].opSample); 
	vertlist[11] = vertexInterp(iVertTest, vert[3].vert, vert[3].sample, vert[3].opVert, vert[3].opSample); 
	
	
	
		vec4 tri[3];
		int i = 0;
		
	//for (i=0; triTableValue(iFlagIndex, i)!=-1; i+=3) { //Strange bug with this way, uncomment to test 
		
	
	while(true){ 
		//if (i>15)
		//	break;
		if(triTableValue(iFlagIndex, i)!=-1){ 
			
			tri[2] = vec4(vertlist[triTableValue(iFlagIndex, i)]); 
			tri[1] = vec4(vertlist[triTableValue(iFlagIndex, i+1)]); 
			tri[0] = vec4(vertlist[triTableValue(iFlagIndex, i+2)]); 
		
			normal = vec4(normalize(cross(vec3(tri[2] - tri[0] ),
								vec3(tri[2] - tri[1] ))),0);
								
	
								
	
		
			gl_Position = tri[0];	
			EmitVertex();
			gl_Position = tri[1];	
			EmitVertex();
			gl_Position = tri[2];	
			EmitVertex();
		
			//End triangle strip at firts triangle 
			EndPrimitive(); 
		}else{ 
			break; 
		} 
 
		i=i+3; //Comment it to test the strange bug 
	//	break;
	} 
		
		
		
		
		
		
		
		
		
		
	/*
		
		
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
		
		*/
 }