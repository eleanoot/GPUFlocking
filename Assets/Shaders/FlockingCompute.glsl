#version 430 core

uniform float sep_dist = 50.0;
uniform float sep_weight = 1.5;
uniform float align_weight = 1.0;
uniform float coh_weight = 1.0;
uniform vec3 goal = vec3(0.0);

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

struct flock_member
{
	vec3 position;
	vec3 velocity;
	vec3 accel;
};

layout(std430, binding = 0) buffer members_in
{
	flock_member member[];
} input_data;

//layout (std430, binding = 1) buffer members_out
//{
//	flock_member member[];
//} output_data;

shared flock_member shared_member[gl_WorkGroupSize.x];

vec3 separation(vec3 my_pos, vec3 my_vel, vec3 their_pos, vec3 their_vel)
{
	vec3 d = my_pos - their_pos;
	if (dot(d,d) < sep_dist)
		return d;
	return vec3(0.0);
}

vec3 stable(vec3 my_pos, vec3 my_vel, vec3 their_pos, vec3 their_vel)
{
	vec3 d = their_pos - my_pos;
	vec3 dv = their_vel - my_vel;
	return dv / (dot(d,d) + 10.0);
}

void main(void)
{
	int global_id = int(gl_GlobalInvocationID.x);
	int local_id = int(gl_LocalInvocationID.x);
	
	flock_member me = input_data.member[global_id];
	flock_member new_me;
	
	vec3 accel = vec3(0.0);
	vec3 flock_centre = vec3(0.0);
	
	for (int i = 0; i < gl_NumWorkGroups.x; i++)
	{
		flock_member them = input_data.member[i * gl_WorkGroupSize.x + local_id];
		shared_member[local_id] = them;
		memoryBarrierShared();
		barrier();
		for (int j = 0; j < gl_WorkGroupSize.x; j++)
		{
			them = shared_member[j];
			flock_centre += them.position;
			if (i * gl_WorkGroupSize.x + j != global_id)
			{
				accel += separation(me.position, me.velocity, them.position, them.velocity) * sep_weight;
				accel += stable(me.position, me.velocity, them.position, them.velocity);
			}
		}
		barrier();
	}
	
	flock_centre /= float(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	new_me.position = me.position + me.velocity;
	accel += normalize(goal - me.position) * align_weight;
	accel += normalize(flock_centre - me.position) * coh_weight;
	new_me.velocity = me.velocity + accel;
	new_me.velocity = mix(me.velocity, new_me.velocity, 0.4);
	output_data.member[global_id] = new_me;
		
	
}