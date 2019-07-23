#version 420 compatibility

#pragma optimize(on)

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

layout( quads ) in;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 nm;

// Vertex position in camera coords
out vec4 tess_position;
// Vertex normal in camera coords.
out vec4 tess_normal;

// Calculates Bernstein polynomials
void bernstein(out float[4] b, out float[4] db, float t) {
	float t1 = (1.0 - t);
	float t12 = t1 * t1;
	// Polynomials
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

vec4 bezier(in vec4[16] cp,in float[4] bu, in float[4] bv) {
	return
		(cp[0]*bu[0]*bv[0]) + (cp[1]*bu[0]*bv[1]) + (cp[2]*bu[0]*bv[2]) +
		(cp[3]*bu[0]*bv[3]) +
		(cp[4]*bu[1]*bv[0]) + (cp[5]*bu[1]*bv[1]) + (cp[6]*bu[1]*bv[2]) +
		(cp[7]*bu[1]*bv[3]) +
		(cp[8]*bu[2]*bv[0]) + (cp[9]*bu[2]*bv[1]) + (cp[10]*bu[2]*bv[2]) +
		(cp[11]*bu[2]*bv[3]) +
		(cp[12]*bu[3]*bv[0]) + (cp[13]*bu[3]*bv[1]) + (cp[14]*bu[3]*bv[2]) +
		(cp[15]*bu[3]*bv[3]);
}


void main(void) {
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	// Copy sixteen control points
	vec4 control_points[16];
	for(int i=0; i < 16; i++) {
		control_points[i] = gl_in[i].gl_Position;
	}

	// basis for u and v
	float bu[4];
	float bv[4];
	// Derivitives for u and v
	float dbu[4];
	float dbv[4];

	bernstein(bu, dbu, u);
	bernstein(bv, dbv, v);

	// Bezier spline interpolation
	tess_position = bezier(control_points, bu, bv);

	// The partial derivatives, for calculating normal vector
	vec4 du = bezier(control_points, dbu, bv);
	vec4 dv = bezier(control_points, bu, dbv);

	// The normal is the cross product of the partials
	vec3 normal = normalize( cross(du.xyz, dv.xyz) );
	// Transform to clip coordinates
	gl_Position = mvp * tess_position;
	tess_position = mv * tess_position;
	tess_normal = normalize( nm * vec4(normal,0.0) );
}
