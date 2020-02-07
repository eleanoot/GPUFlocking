#version 430 core

layout(std430, binding = 0) buffer ColourBuffer
{
	vec4[] colourData;
} colourBuffer;

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

void main(void)
{
	uint gid = gl_GlobalInvocationID.x;

	if (gid % 2 == 0)
	{
		colourBuffer.colourData[gid].x = 0;
		colourBuffer.colourData[gid].y = 1;
		colourBuffer.colourData[gid].z = 0;
		colourBuffer.colourData[gid].w = 1;
	}
	else
	{
		colourBuffer.colourData[gid].x = 0;
		colourBuffer.colourData[gid].y = 0;
		colourBuffer.colourData[gid].z = 1;
		colourBuffer.colourData[gid].w = 1;
	}
	
}