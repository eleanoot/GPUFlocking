#version 430 core

layout(std430, binding = 0) buffer ColourBuffer
{
	vec4[] colourData;
} colourBuffer;

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

void main(void)
{
	
	
}