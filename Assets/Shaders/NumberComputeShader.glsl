#version 430 core

layout(binding = 4) buffer Nums
{
	int data[];
};

layout(local_size_x = 100, local_size_y = 1, local_size_z = 1) in;

void main(void)
{
	uint gid = gl_GlobalInvocationID.x;

	data[gid] = 5;
}