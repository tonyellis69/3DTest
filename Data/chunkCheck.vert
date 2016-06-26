#version 330

layout(location = 0) in vec4 position;

uniform int face;
smooth out vec3 v_texCoord3D;




void main(  ) {
	gl_Position = position;// Just passes on the given vertex coordinate to OGL.
	vec2 normPosition = (position.xy + vec2(1,1))  * 0.5; //convert vertex position to the range 0-1
	if (face == 0) //base of cube
		v_texCoord3D = vec3(normPosition.x,0,normPosition.y); //passes position to frag shader as a 3D texture coordinate on the xz plane.
	
	else if (face == 1) //top of cube
		v_texCoord3D = vec3(normPosition.x,1,normPosition.y); //passes position to frag shader as a 3D texture coordinate on the xz plane.
	
	else if (face == 2) //south face
		v_texCoord3D = vec3(normPosition.x,normPosition.y,1); //passes position to frag shader as a 3D texture coordinate on the xy plane.
		
		
	else if (face == 3) //east face
		v_texCoord3D = vec3(1,normPosition.y,normPosition.x); //passes position to frag shader as a 3D texture coordinate on the yz plane.
		
	else if (face == 4) //north face
		v_texCoord3D = vec3(normPosition.x,normPosition.y,0); //passes position to frag shader as a 3D texture coordinate on the xy plane.
		
	else if (face == 5) //west face
		v_texCoord3D = vec3(0,normPosition.y,normPosition.x); //passes position to frag shader as a 3D texture coordinate on the yz plane.
}
