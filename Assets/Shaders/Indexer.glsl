#version 430 core

uniform int numBoids;
uniform float ratio;
uniform uvec2 cellCounts;
uniform int cellCount;

struct flock_member
{
	vec3 pos;
	float angle;
	vec3 vel;
	float groupNo;
	vec3 accel;
	float scrap3;
};


layout(std140, binding = 0) buffer Flock_In
{
	flock_member input_flock[];
};

layout(std430, binding = 6) buffer Boid_Offsets
{
	uint offsets[];
};

layout(std430, binding = 8) buffer Boid_Indexes
{
	uint indexes[];
};

layout( local_size_x = 128, local_size_y = 1, local_size_z = 1 ) in;

void main()
{
	uint gid = gl_GlobalInvocationID.x;

	if (gid >= numBoids) return;

	uvec2 cell = uvec2(input_flock[gid].pos.xz * ratio);
	uint cellNum = (cell.x + cell.y * cellCounts.x) % cellCount;

	uint offset = atomicAdd(offsets[cellNum], 1);
	indexes[offset] = gid;
	
}