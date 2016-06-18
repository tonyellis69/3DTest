#version 420

layout(location = 0) in vec4 position;


uniform vec4 cubeScale;
uniform vec4 inColour;
uniform vec3 samplePos; //position of bottom nwcorner of this chunk in sample space.
uniform float hFactor; //Vertical adjustment.

out VertexData {
    vec4 corner[8];
	float cornerSample[8];
} outData;

out vec4 vColour;




vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v)
  {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}


float octave_noise_2d(  float octaves,  float persistence,  float scale,  float x,  float y ) {
    float total = 0;
    float frequency = scale;
    float amplitude = 1;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
    float maxAmplitude = 0;

    for( int i=0; i < octaves; i++ ) {
        total += snoise(vec2( x * frequency, y * frequency )) * amplitude;

        frequency *= 2;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }

    return total / maxAmplitude;
}

float getSample(vec4 corner) {
	corner = position + corner;
	
	//1. get the surface height at the 2D position of this corner.
	float surfaceHeight = octave_noise_2d(6,0.2,0.02,corner.x,corner.z);
	surfaceHeight = (surfaceHeight * 0.5) + 0.5;  //convert to 0 - 1.
	
	
	//2. scale the surface height up to lie somewhere in sample space.
	surfaceHeight *= hFactor;
	
	
	//3. clip the surface height against the height of this corner. Values outside 1 mean the surface doesn't intersect this point.
	float sampleVal = corner.y - surfaceHeight;
	
	//4. we now have a sample value for this corner.
	return sampleVal;
}



void main()
{
	//gl_Position = mvpMatrix  * (position * cubeScale) ;
	vColour = inColour;
	
	outData.corner[0] =  ((position + vec4(-0.3,-0.3,-0.3,0)) * cubeScale) ;
	outData.corner[1] =  ((position + vec4(0.3,-0.3,-0.3,0)) * cubeScale) ;
	outData.corner[2] =  ((position + vec4(0.3,-0.3,0.3,0)) * cubeScale) ;
	outData.corner[3] =  ((position + vec4(-0.3,-0.3,0.3,0)) * cubeScale) ;
	
	
	outData.corner[4] =  ((position + vec4(-0.3,0.3,-0.3,0)) * cubeScale) ;
	outData.corner[5] =  ((position + vec4(0.3,0.3,-0.3,0)) * cubeScale) ;
	outData.corner[6] =  ((position + vec4(0.3,0.3,0.3,0)) * cubeScale) ;
	outData.corner[7] = ((position + vec4(-0.3,0.3,0.3,0)) * cubeScale) ;
	
	outData.cornerSample[0] =  getSample(vec4(-0.5,-0.5,-0.5,0));
	outData.cornerSample[1] =  getSample(vec4(0.5,-0.5,-0.5,0));
	outData.cornerSample[2] =  getSample(vec4(0.5,-0.5,0.5,0));
	outData.cornerSample[3] =  getSample(vec4(-0.5,-0.5,0.5,0));
	
	
	outData.cornerSample[4] =  getSample(vec4(-0.5,0.5,-0.5,0));
	outData.cornerSample[5] =  getSample(vec4(0.5,0.5,-0.5,0));
	outData.cornerSample[6] =  getSample(vec4(0.5,0.5,0.5,0));
	outData.cornerSample[7] =  getSample(vec4(-0.5,0.5,0.5,0));
	
	
}
