#version 410

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec2 fragUv[];
layout(location = 1) in vec3 poss[];
layout(location = 2) in vec4 colorss[];

layout(location = 0) out vec2 UV;
layout(location = 1) out vec3 pos;
layout(location = 2) out vec3 norm;

layout(location = 3) noperspective out vec3 dist; 

layout(location = 4) out vec4 color;

const vec2 WIN_SCALE = vec2(1280, 720);

void main() {

	vec2 p0 = WIN_SCALE * gl_in[0].gl_Position.xy/gl_in[0].gl_Position.w;
	vec2 p1 = WIN_SCALE * gl_in[1].gl_Position.xy/gl_in[1].gl_Position.w;
	vec2 p2 = WIN_SCALE * gl_in[2].gl_Position.xy/gl_in[2].gl_Position.w;
	vec2 v0 = p2-p1;
	vec2 v1 = p2-p0;
	vec2 v2 = p1-p0;
	
	float area = abs(v1.x*v2.y - v1.y * v2.x) ;
	
	norm = normalize(cross(poss[1] - poss[0], poss[2] - poss[0]));
	
	UV = fragUv[0];
	dist = vec3(area/length(v0),0,0);
	gl_Position = gl_in[0].gl_Position;
	gl_ClipDistance[0] = gl_in[0].gl_ClipDistance[0];
	pos = poss[0];
	color = colorss[0];
	EmitVertex();

	UV = fragUv[1];
	dist = vec3(0,area/length(v1),0);
	gl_Position = gl_in[1].gl_Position;
	gl_ClipDistance[0] = gl_in[1].gl_ClipDistance[0];
	pos = poss[1];
	color = colorss[1];
	EmitVertex();

	UV = fragUv[2];
	dist = vec3(0,0,area/length(v2));
	gl_Position = gl_in[2].gl_Position;
	gl_ClipDistance[0] = gl_in[2].gl_ClipDistance[0];
	pos = poss[2];
	color = colorss[2];
	EmitVertex();
	EndPrimitive();

}