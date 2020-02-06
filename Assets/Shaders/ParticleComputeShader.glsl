#version 430 core

layout(std140, binding = 4) buffer Pos
{
	vec4 Positions[];
};

layout(std140, binding = 5) buffer Vel
{
	vec4 Velocities[];
};

layout(std140, binding = 6) buffer Col
{
	vec4 Colours[];
};

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

void main(void)
{
	const vec3 G = vec3(0, -9.8, 0);
	const float DT = 0.1;

	uint gid = gl_GlobalInvocationID.x;

	vec3 p = Positions[gid].xyz;
	vec3 v = Velocities[gid].xyz;
	
	vec3 pp = p + v * DT + 0.5 * DT * DT * G;
	vec3 vp = v + G * DT;

	Positions[gid].xyz = pp;
	Velocities[gid].xyz = vp;
}