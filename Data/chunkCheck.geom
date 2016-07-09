#version 330


 layout(points) in;
 layout(triangle_strip, max_vertices = 3) out;

//The sample value[s] for the vertex[es] passed to this shader.

in VertexData {
	float vertSample;
} vert[];


void main() {
	if ((vert[0].vertSample < 1) && (vert[0].vertSample > 0)) {
		EmitVertex();
		EmitVertex();
		EmitVertex();
		EndPrimitive();
	}
	else {
	
		return;
		}


}