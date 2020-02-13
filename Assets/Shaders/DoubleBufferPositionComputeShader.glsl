#version 430 core

layout(std430, binding = 0) buffer PosBuffer
{
	vec3[] posData;
} posBuffer;

layout(std430, binding = 1) buffer PosBuffer_Out
{
	vec3[] posData;
} posOut;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main(void)
{
	uint gid = gl_GlobalInvocationID.x;

	posOut.posData[gid].z = (posBuffer.posData[gid].z + 0.1);
	
}