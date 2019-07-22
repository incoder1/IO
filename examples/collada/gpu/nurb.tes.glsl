#version 420

#pragma optimize(on)

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

layout( quads ) in;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 mn;


out vec4 tess_position; // Vertex position in camera coords
out vec4 tess_normal; // Vertex normal in camera coords.

void bernstein_polynomials(out float[4] b, out float[4] db, float t) {
	float t1 = (1.0 - t);
	float t12 = t1 * t1;
	
	// Bernstein polynomials
	b[0] = t12 * t1;
	b[1] = 3.0 * t12 * t;
	b[2] = 3.0 * t1 * t * t;
	b[3] = t * t * t;
	
	// Derivatives
	db[0] = -3.0 * t1 * t1;
	db[1] = -6.0 * t * t1 + 3.0 * t12;
	db[2] = -3.0 * t * t + 6.0 * t * t1;
	db[3] = 3.0 * t * t;
}

void main(void) {
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	// The sixteen control points
	vec4 p00 = gl_in[0].gl_Position;
	vec4 p01 = gl_in[1].gl_Position;
	vec4 p02 = gl_in[2].gl_Position;
	vec4 p03 = gl_in[3].gl_Position;
	vec4 p10 = gl_in[4].gl_Position;
	vec4 p11 = gl_in[5].gl_Position;
	vec4 p12 = gl_in[6].gl_Position;
	vec4 p13 = gl_in[7].gl_Position;
	vec4 p20 = gl_in[8].gl_Position;
	vec4 p21 = gl_in[9].gl_Position;
	vec4 p22 = gl_in[10].gl_Position;
	vec4 p23 = gl_in[11].gl_Position;
	vec4 p30 = gl_in[12].gl_Position;
	vec4 p31 = gl_in[13].gl_Position;
	vec4 p32 = gl_in[14].gl_Position;
	vec4 p33 = gl_in[15].gl_Position;
	
	// basis for u and v
	float bu[4];
	float bv[4]; 
	 // Derivitives for u and v
	float dbu[4];
	float dbv[4];
	bernstein_polynomials(bu, dbu, u);
	bernstein_polynomials(bv, dbv, v);
	
	// Bezier interpolation
	tess_position = 
		p00*bu[0]*bv[0] + p01*bu[0]*bv[1] + p02*bu[0]*bv[2] +
		p03*bu[0]*bv[3] +
		p10*bu[1]*bv[0] + p11*bu[1]*bv[1] + p12*bu[1]*bv[2] +
		p13*bu[1]*bv[3] +
		p20*bu[2]*bv[0] + p21*bu[2]*bv[1] + p22*bu[2]*bv[2] +
		p23*bu[2]*bv[3] +
		p30*bu[3]*bv[0] + p31*bu[3]*bv[1] + p32*bu[3]*bv[2] +
		p33*bu[3]*bv[3];
	// The partial derivatives
	vec4 du =
		p00*dbu[0]*bv[0]+p01*dbu[0]*bv[1]+p02*dbu[0]*bv[2]+
		p03*dbu[0]*bv[3]+
		p10*dbu[1]*bv[0]+p11*dbu[1]*bv[1]+p12*dbu[1]*bv[2]+
		p13*dbu[1]*bv[3]+
		p20*dbu[2]*bv[0]+p21*dbu[2]*bv[1]+p22*dbu[2]*bv[2]+
		p23*dbu[2]*bv[3]+
		p30*dbu[3]*bv[0]+p31*dbu[3]*bv[1]+p32*dbu[3]*bv[2]+
		p33*dbu[3]*bv[3];
		
	vec4 dv =
		p00*bu[0]*dbv[0]+p01*bu[0]*dbv[1]+p02*bu[0]*dbv[2]+
		p03*bu[0]*dbv[3]+
		p10*bu[1]*dbv[0]+p11*bu[1]*dbv[1]+p12*bu[1]*dbv[2]+
		p13*bu[1]*dbv[3]+
		p20*bu[2]*dbv[0]+p21*bu[2]*dbv[1]+p22*bu[2]*dbv[2]+
		p23*bu[2]*dbv[3]+
		p30*bu[3]*dbv[0]+p31*bu[3]*dbv[1]+p32*bu[3]*dbv[2]+
		p33*bu[3]*dbv[3];
	// The normal is the cross product of the partials
	vec3 normal = normalize( cross(du.xyz, dv.xyz) );
	// Transform to clip coordinates
	gl_Position = mvp * tess_position;
	tess_position = mv * tess_position;
	tess_normal = normalize( mn * vec4(normal,0.0) );
}