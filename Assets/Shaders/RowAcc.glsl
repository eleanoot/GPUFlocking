#version 430 core

uniform uvec2 cellCounts;

layout(std430, binding = 6) buffer BoidOffsets 
{
	uint boid_offsets[];
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
	uint acc = 0;
	for (uint i = 0; i < cellCounts.x; i++)
	{
		row_acc[i] = acc;
		acc += row_counts[i];
	}
}