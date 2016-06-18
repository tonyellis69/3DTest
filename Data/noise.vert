#version 330

layout(location = 0) in vec4 position;

smooth out vec3 v_texCoord3D;

void main(  ) {
	gl_Position.xy = position.xy;// Just passes on the given vertex coordinate to OGL.
	vec2 normPosition = (position.xy + vec2(1,1))  * 0.5; //convert vertex position to the range 0-1
	v_texCoord3D = vec3(normPosition.x,0,normPosition.y);; //passes position to frag shader as a 3D texture coordinate on the xz plane.
}
