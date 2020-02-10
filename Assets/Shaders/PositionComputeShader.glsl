#version 430 core

layout(std430, binding = 0) buffer PosBuffer
{
	vec3[] posData;
} posBuffer;

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

void main(void)
{
	uint gid = gl_GlobalInvocationID.x;

	posBuffer.posData[gid].z += 0.01;
	
}