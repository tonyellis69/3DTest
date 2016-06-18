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
		//if (vert[0].cornerSample[0] > 0.5)
		//	return;

		
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