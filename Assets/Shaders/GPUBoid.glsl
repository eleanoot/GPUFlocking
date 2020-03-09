#version 430 core

uniform float sepDis;
uniform float alignDis;
uniform float cohDis;

uniform float sepWeight;
uniform float alignWeight;
uniform float cohWeight;

uniform float maxSpeed;
uniform float maxForce;

uniform float dt;

struct flock_member
{
	vec3 pos;
	float scrap1;
	vec3 vel;
	float scrap2;
	vec3 accel;
	float scrap3;
};

layout(std140, binding = 0) buffer Flock_In
{
	flock_member input_flock[];
};

layout(std140, binding = 1) buffer Flock_Out
{
	flock_member output_flock[];
};

layout( local_size_x = 256, local_size_y = 1, local_size_z = 1 ) in;

vec3 Limit(vec3 v, float m)
{
	float lengthSqr = v.x * v.x + v.y * v.y + v.z * v.z;

	if (lengthSqr > m*m && lengthSqr > 0.0)
	{
		float ls = sqrt(lengthSqr);
		float ratio = m / ls;
		v *= ratio;
	}
	return v;
}

vec3 Seek(vec3 pos, vec3 vel, vec3 target)
{
	vec3 desired = target;
	desired -= pos; 
	desired = normalize(desired); 
	desired *= maxSpeed;
	uint gid = gl_GlobalInvocationID.x;
	output_flock[gid].accel = desired - vel;
	Limit(output_flock[gid].accel, maxForce);
	return output_flock[gid].accel;
	//vec3 steer = desired;
	//desired -= vel;
	//desired = Limit(steer, maxSpeed);
	//return steer;
}

vec3 Separation(vec3 pos, vec3 vel)
{
	uint localID = gl_LocalInvocationID.x;

	vec3 steering = vec3(0.0,0.0,0.0);
	float count = 0.0;

	for (uint i = 0; i < gl_NumWorkGroups.x; i++)
	{
		if (i == gl_GlobalInvocationID.x)
			continue;
		//vec3 otherPos = input_flock[i * gl_WorkGroupSize.x + localID].pos;
		vec3 otherPos = input_flock[i].pos;
		float d = abs(distance(pos, otherPos));
		
		if(d < sepDis && d > 0.0)
		{
			vec3 diff = vec3(0,0,0);
			diff = pos - otherPos;
			diff = normalize(diff);
			diff /= d;
			steering += diff;
			count += 1.0;
		}
	}	


	if (count > 0.0)
	{
		steering /= count;
	}

	if(length(steering) > 0.0)
	{
		steering = normalize(steering);
		steering *= maxSpeed;
		steering -= vel;
		steering = Limit(steering, maxForce);
	}

	//steering.y = count;
	return steering;
}

vec3 Alignment(vec3 pos, vec3 vel)
{
	uint localID = gl_LocalInvocationID.x;

	vec3 sum = vec3(0.0,0.0,0.0);
	float count = 0.0;

	for (uint i = 0; i < gl_NumWorkGroups.x; i++)
	{
		if (i == gl_GlobalInvocationID.x)
			continue;
		vec3 otherPos = input_flock[i].pos;
		float d = abs(distance(pos, otherPos));

		if (d < alignDis && d > 0.0)
		{
			sum += input_flock[i].vel;
			count += 1.0;
		}
	}

	if (count > 0.0)
	{
		sum /= count;
		sum = normalize(sum);
		sum *= maxSpeed;

		vec3 steering = sum - vel;
		steering = Limit(steering, maxForce);

		//steering = vec3(0, count, 0);
		return steering;
	}
	else
	{
		return vec3(0.0,0.0,0.0);
	}
}

vec3 Cohesion(vec3 pos, vec3 vel)
{
	uint localID = gl_LocalInvocationID.x;

	vec3 steering = vec3(0.0,0.0,0.0);
	float count = 0.0;
	for (uint i = 0; i < gl_NumWorkGroups.x; i++)
	{
		if (i == gl_GlobalInvocationID.x)
			continue;
		vec3 otherPos = input_flock[i].pos;
		float d = abs(distance(pos, otherPos));

		if (d < cohDis && d > 0.0)
		{
			steering += otherPos;
			count += 1.0;
		}
	}

	if(count > 0.0)
	{
		steering /= count;
		return Seek(pos, vel, steering);
	}
	else
	{
		return vec3(0.0,0.0,0.0);
	}
}

vec3 ApplyForce(vec3 accel, vec3 force)
{
	return accel + force;
} 

vec3 Update(vec3 pos, vec3 vel, vec3 accel)
{
	vec3 sep = Separation(pos, vel);
	vec3 align = Alignment(pos, vel);
	vec3 cohesion = Cohesion(pos, vel);

	sep *= sepWeight;
	align *= alignWeight;
	cohesion *= cohWeight;

	accel = ApplyForce(accel, sep);
	accel = ApplyForce(accel, align);
	accel = ApplyForce(accel, cohesion);

	accel *= 0.4;
	vel += accel * dt;
	Limit(vel, maxSpeed);
	//pos += vel * dt;

	////pos.y = align.y;

	///*vel = Limit(vel, maxSpeed);
	//pos += vel;*/

	//if (pos.x < -1010)
	//	pos.x += 2000;
	//if (pos.z < -1010)
	//	pos.z += 2000;

	//if (pos.x > 1010)
	//	pos.x -= 2000;
	//if (pos.z > 1010)
	//	pos.z -= 2000;

	return vel;
}


void main()
{
	uint gid = gl_GlobalInvocationID.x;
	uint lid = gl_LocalInvocationID.x;
	flock_member thisMember = input_flock[gid];

	//vec3 newPos = Update(thisMember.pos, thisMember.vel, thisMember.accel);
	vec3 newVel = Update(thisMember.pos, thisMember.vel, thisMember.accel);
	input_flock[gid].accel = vec3(0,0,0);
	output_flock[gid].accel = vec3(0,0,0);

	vec3 pos = thisMember.pos;
	pos += newVel * dt;

	if (pos.x < -1010)
		pos.x += 2000;
	if (pos.z < -1010)
		pos.z += 2000;

	if (pos.x > 1010)
		pos.x -= 2000;
	if (pos.z > 1010)
		pos.z -= 2000;

	output_flock[gid].vel = newVel * 0.999;
	output_flock[gid].pos = pos;
	/*output_flock[gid].accel = input_flock[gid].accel + vec3(0.00002, 0, 0);
	output_flock[gid].vel = input_flock[gid].vel + output_flock[gid].accel;
	output_flock[gid].pos = input_flock[gid].pos + output_flock[gid].vel;*/

}