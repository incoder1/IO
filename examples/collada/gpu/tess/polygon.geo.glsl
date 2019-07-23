#version 420 compatibility

#pragma optimize(on)

#ifdef GL_ES
precision mediump float;
#else
precision highp float;
#endif

layout( triangles ) in;
layout( triangle_strip, max_vertices = 3 ) out;

uniform mat4 vpm;

in vec3 vertex_position;
in vec3 vertex_normal;

out vec3 geom_position;
out vec3 geom_normal;

noperspective out vec3 edge_distance;

void main(void) {
	vec3 p0 = vec3(vpm * (gl_in[0].gl_Position / gl_in[0].gl_Position.w) );
	vec3 p1 = vec3(vpm * (gl_in[1].gl_Position / gl_in[1].gl_Position.w) );
	vec3 p2 = vec3(vpm * (gl_in[2].gl_Position / gl_in[2].gl_Position.w) );
	
	float a = length(p1 - p2);
	float b = length(p2 - p0);
	float c = length(p1 - p0);
	
	float alpha = acos( (b*b + c*c - a*a) / (2.0*b*c) );
	float beta = acos( (a*a + c*c - b*b) / (2.0*a*c) );

	float ha = abs( c * sin( beta ) );
	float hb = abs( c * sin( alpha ) );
	float hc = abs( b * sin( alpha ) );
	
	edge_distance = vec3( ha, 0.0, 0.0 );
	geom_normal = vertex_normal[0];
	geom_position = vertex_position[0];
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	
	edge_distance = vec3( 0.0, hb, 0.0 );
	geom_normal = vertex_normal[1];
	geom_position = vertex_position[1];
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	edge_distance = vec3( 0.0, 0.0, hc );
	geom_normal = vertex_normal[2];
	geom_position = vertex_position[2];
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
	
	EndPrimitive();
	
}
