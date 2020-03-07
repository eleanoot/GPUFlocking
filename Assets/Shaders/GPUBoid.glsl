#version 430 core

uniform float sepDis;
uniform float alignDis;
uniform float cohDis;

uniform float sepWeight;
uniform float alignWeight;
uniform float cohWeight;

uniform float maxSpeed;

struct flock_member
{
	vec3 pos;
	vec3 vel;
};

layout(std140, binding = 0) buffer Flock
{
	flock_member flock[];
};

layout( local_size_x = 128, local_size_y = 1, local_size_z = 1 ) in;

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

	vec3 steer = desired;
	desired -= vel;
	desired = Limit(steer, maxSpeed);
	return steer;
}

vec3 Separation(vec3 pos, vec3 vel)
{
	uint localID = gl_LocalInvocationID.x;

	vec3 steering = vec3(0.0,0.0,0.0);
	float count = 0.0;

	for (uint i = 0; i < gl_NumWorkGroups.x; i++)
	{
		vec3 otherPos = flock[i * gl_WorkGroupSize.x + localID].pos;
		float d = abs(distance(pos, otherPos));

		if(d < sepDis && d > 0.0)
		{
			vec3 diff = pos;
			diff -= otherPos;
			diff = normalize(diff);
			diff *= d;
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
		steering = Limit(steering, 1.0);
	}

	return steering;
}

vec3 Alignment(vec3 pos, vec3 vel)
{
	uint localID = gl_LocalInvocationID.x;

	vec3 sum = vec3(0.0,0.0,0.0);
	float count = 0.0;

	for (uint i = 0; i < gl_NumWorkGroups.x; i++)
	{
		vec3 otherPos = flock[i * gl_WorkGroupSize.x + localID].pos;
		float d = abs(distance(pos, otherPos));

		if (d < alignDis && d > 0.0)
		{
			sum += flock[i * gl_WorkGroupSize.x + localID].vel;
			count += 1.0;
		}
	}

	if (count > 0.0)
	{
		sum /= count;
		sum = normalize(sum);
		sum *= maxSpeed;

		vec3 steering = sum - vel;
		steering = Limit(steering, maxSpeed);
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
		vec3 otherPos = flock[i * gl_WorkGroupSize.x + localID].pos;
		float d = abs(distance(pos, otherPos));

		if (d < cohDis)
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

vec3 ApplyForce(vec3 vel, vec3 force)
{
	return vel + force;
}

vec3 Update(vec3 pos, vec3 vel)
{
	vec3 sep = Separation(pos, vel);
	vec3 align = Alignment(pos, vel);
	vec3 cohesion = Cohesion(pos, vel);

	sep *= sepWeight;
	align *= alignWeight;
	cohesion *= cohWeight;

	vel = ApplyForce(vel, sep);
	vel = ApplyForce(vel, align);
	vel = ApplyForce(vel, cohesion);

	vel = Limit(vel, maxSpeed);
	pos += vel;

	return pos;
}


void main()
{
	uint gid = gl_GlobalInvocationID.x;
	uint lid = gl_LocalInvocationID.x;
	flock_member thisMember = flock[gid];

	vec3 newPos = Update(thisMember.pos, thisMember.vel);

	//flock[gid].pos = newPos;
	flock[gid].pos = thisMember.pos += 10;

}