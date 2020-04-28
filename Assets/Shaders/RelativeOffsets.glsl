#version 430 core

uniform int numBoids;
uniform uvec2 cellCounts;

layout(std430, binding = 6) buffer BoidOffsets 
{
	uint boid_offsets[];
};

layout(std430, binding = 7) buffer Atomic_Offsets
{
	uint atOffsets[];
};

layout(std430, binding = 9) buffer GridRowCounts
{
	uint row_counts[];
};

layout(std430, binding = 10) buffer RowAccumulate
{
	uint row_acc[];
};

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in; /// ???

void main()
{
	// acting with one invocation per cell 
	uint gid = gl_GlobalInvocationID.x; // cell/offset index we're looking at

	if (gid >= numBoids) return;

	// determine which row this cell is on
	uint rowNo = gid / cellCounts.x;

	atOffsets[gid] = rowNo;


}