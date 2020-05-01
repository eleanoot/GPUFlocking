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

layout(local_size_x = 50, local_size_y = 1, local_size_z = 1) in; /// ???

void main()
{
	// one invocation per row
	uint i = gl_GlobalInvocationID.x * cellCounts.x;  // index to start from
	uint last = i + cellCounts.x;

	for (; i < last; i++)
	{
		// for every cell, add the accumulated sum for the row it's on 
		boid_offsets[i] += row_acc[gl_GlobalInvocationID.x];
		atOffsets[i] = boid_offsets[i];
	}

}