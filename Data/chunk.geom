#version 330



#extension GL_EXT_geometry_shader4 : enable 
#extension GL_EXT_gpu_shader4 : enable 
 

 layout(points) in;
 layout(triangle_strip, max_vertices = 16) out;

in VertexData
{
    vec4 corner[8];
	float cornerSample[8];
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
		
		
		
		
		
		
	vec4 vertlist[12]; 
	 
	//Find the vertices where the surface intersects the cube 
	vertlist[0] = vertexInterp(iVertTest, vert[0].corner[0], vert[0].cornerSample[0], vert[0].corner[1], vert[0].cornerSample[1]); 
	vertlist[1] = vertexInterp(iVertTest, vert[0].corner[1], vert[0].cornerSample[1], vert[0].corner[2], vert[0].cornerSample[2]); 
	vertlist[2] = vertexInterp(iVertTest, vert[0].corner[2], vert[0].cornerSample[2], vert[0].corner[3], vert[0].cornerSample[3]); 
	vertlist[3] = vertexInterp(iVertTest, vert[0].corner[3], vert[0].cornerSample[3], vert[0].corner[0], vert[0].cornerSample[0]); 
	vertlist[4] = vertexInterp(iVertTest, vert[0].corner[4], vert[0].cornerSample[4], vert[0].corner[5], vert[0].cornerSample[5]); 
	vertlist[5] = vertexInterp(iVertTest, vert[0].corner[5], vert[0].cornerSample[5], vert[0].corner[6], vert[0].cornerSample[6]); 
	vertlist[6] = vertexInterp(iVertTest, vert[0].corner[6], vert[0].cornerSample[6], vert[0].corner[7], vert[0].cornerSample[7]); 
	vertlist[7] = vertexInterp(iVertTest, vert[0].corner[7], vert[0].cornerSample[7], vert[0].corner[4], vert[0].cornerSample[4]); 
	vertlist[8] = vertexInterp(iVertTest, vert[0].corner[0], vert[0].cornerSample[0], vert[0].corner[4], vert[0].cornerSample[4]); 
	vertlist[9] = vertexInterp(iVertTest, vert[0].corner[1], vert[0].cornerSample[1], vert[0].corner[5], vert[0].cornerSample[5]); 
	vertlist[10] = vertexInterp(iVertTest, vert[0].corner[2], vert[0].cornerSample[2], vert[0].corner[6], vert[0].cornerSample[6]); 
	vertlist[11] = vertexInterp(iVertTest, vert[0].corner[3], vert[0].cornerSample[3], vert[0].corner[7], vert[0].cornerSample[7]); 
	
	
	
		vec4 tri[3];
		int i = 0;
		
	//for (i=0; triTableValue(iFlagIndex, i)!=-1; i+=3) { //Strange bug with this way, uncomment to test 
		
	
	while(true){ 
		if (i>15)
			break;
		if(triTableValue(iFlagIndex, i)!=-1){ 
			
			tri[2] = vec4(vertlist[triTableValue(iFlagIndex, i)]); 
			tri[1] = vec4(vertlist[triTableValue(iFlagIndex, i+1)]); 
			tri[0] = vec4(vertlist[triTableValue(iFlagIndex, i+2)]); 
		
			normal = vec4(normalize(cross(vec3(tri[2] - tri[0] ),
								vec3(tri[2] - tri[1] ))),0);
								
	
								
			//normal.x = texelFetch(triTableTex, ivec2(0,0),0).a;//triTableValue(2, 0);
			//normal.y = texelFetch(triTableTex, ivec2(1,0),0).a;//triTableValue(2, 0);
			//normal.z = texelFetch(triTableTex, ivec2(2,0),0).a;//triTableValue(2, 0);
			//normal.w = iFlagIndex;
			
		//	normal.x = tri[0].x;
		//	normal.y = tri[1].x;
		//	normal.z = tri[2].x;
			
		//	normal.x = triTableValue(0, 0);
		
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