#include "CPUBoid.h"

using namespace NCL;
using namespace CSC8503;

CPUBoid::CPUBoid(float x, float z, OGLMesh* mesh, OGLShader* shader) : GameObject("BOID", false)
{
	accel = Vector3(0, 0, 0);
	vel = Vector3(rand() % 3, 0, rand() % 3);
	pos = Vector3(x, 0, z);
	std::cout << pos << std::endl;
	maxSpeed = 3.5;
	maxForce = 0.5;

	SphereVolume* volume = new SphereVolume(1.0f);
	SetBoundingVolume((CollisionVolume*)volume);

	transform.SetWorldScale(Vector3(10, 10, 10));

	transform.SetWorldPosition(Vector3(x, 0, z));

	SetRenderObject(new RenderObject(&transform, mesh, nullptr, shader));
	SetPhysicsObject(new PhysicsObject(&transform, GetBoundingVolume()));

	renderObject->SetColour(Vector4(rand() % 2, rand() % 2, rand() % 2, 1));

	GetPhysicsObject()->SetInverseMass(1);
	GetPhysicsObject()->InitSphereInertia();
}

void CPUBoid::ApplyForce(Vector3 force)
{
	accel += force;
}

Vector3 CPUBoid::Separation(std::vector<CPUBoid*> boids)
{
	// Field of vision distance
	float sepDis = 60;
	float colourSepDis = 40;
	Vector3 steer = Vector3(0, 0, 0);
	int neighbourCount = 0;

	for (int i = 0; i < boids.size(); i++)
	{
		if (boids[i] != this)
		{
			// Calculate distance from this boid to the one we're looking at
			float distance = (pos - boids[i]->pos).Length();

			// Allow boids to get closer to boids of the same colour 
			if (renderObject->GetColour() == boids[i]->GetRenderObject()->GetColour())
			{
				if (distance > 0 && (distance < colourSepDis))
				{
					Vector3 diff = Vector3(0, 0, 0);
					diff = pos - boids[i]->pos;
					diff.Normalise();
					diff /= distance; // Weight by the distance
					steer += diff;
					neighbourCount++;
				}
			}
			else
			{
				// If this is a boid and it's too close, move away from it
				if (distance > 0 && (distance < sepDis))
				{
					Vector3 diff = Vector3(0, 0, 0);
					diff = pos - boids[i]->pos;
					diff.Normalise();
					diff /= distance; // Weight by the distance
					steer += diff;
					neighbourCount++;
				}
			}
		}
		
	}

	// Add the average difference of location to acceleration 
	if (neighbourCount > 0)
		steer /= (float)neighbourCount;

	if (steer.Length() > 0)
	{
		// Steering = desired - velocity
		steer.Normalise();
		steer *= maxSpeed;
		steer -= vel;
		steer.Limit(maxForce);
	}

	return steer;
}

Vector3 CPUBoid::Alignment(std::vector<CPUBoid*> boids)
{
	float neighDis = 70;

	Vector3 sum = Vector3(0, 0, 0);
	int neighbourCount = 0;

	for (int i = 0; i < boids.size(); i++)
	{
		if (boids[i] != this)
		{
			float distance = (pos - boids[i]->pos).Length();

			if (distance > 0 && distance < neighDis)
			{
				sum += boids[i]->vel;
				neighbourCount++;
			}
		}
		
	}

	if (neighbourCount > 0)
	{
		sum /= (float)neighbourCount;
		sum.Normalise();
		sum *= maxSpeed;
		Vector3 steer;
		steer = sum - vel;
		steer.Limit(maxForce);
		return steer;
	}
	else
		return Vector3(0, 0, 0);
}

Vector3 CPUBoid::Cohesion(std::vector<CPUBoid*> boids)
{
	float neighDis = 25;
	Vector3 sum(0, 0, 0);
	int neighbourCount = 0;

	for (int i = 0; i < boids.size(); i++)
	{
		if (boids[i] != this)
		{
			float distance = (pos - boids[i]->pos).Length();

			// Boids are more attracted to boids of the same colour

			if (distance > 0 && distance < neighDis)
			{
				if (renderObject->GetColour() == boids[i]->GetRenderObject()->GetColour())
				{
					sum += boids[i]->pos * 3.5f;
				}
				else
					sum += boids[i]->pos;
				
				neighbourCount++;
			}
		}
		
	}

	if (neighbourCount > 0)
	{
		sum /= neighbourCount;
		return Seek(sum);
	}
	else
		return Vector3(0, 0, 0);
}

Vector3 CPUBoid::Seek(Vector3 v)
{
	Vector3 desired;
	desired -= v; // vector from location to target
	desired.Normalise();
	desired *= maxSpeed;
	accel = desired - vel;
	accel.Limit(maxForce);
	return accel;
}

void CPUBoid::Update(std::vector<CPUBoid*> boids)
{
	Vector3 sep = Separation(boids);
	Vector3 align = Alignment(boids);
	Vector3 cohesion = Cohesion(boids);

	// Random weighing right now
	sep *= 2.5;
	align *= 1.0;
	cohesion *= 1.0;

	// Add the force vectors to acceleration 
	ApplyForce(sep);
	ApplyForce(align);
	ApplyForce(cohesion);

	accel *= 0.4;
	tempAccel = accel;
	vel += accel;
	vel.Limit(maxSpeed);
	pos += vel;
	accel *= 0;

	Boundaries();
}

float CPUBoid::Angle(Vector3 v)
{
	// Adjusting the pitch of the boid
	return (float)(atan2(v.x, v.z) * 180 / 3.14);
}

// If the boids loop off the edge of the floor, wrap them around to the other side to not go off screen.
void CPUBoid::Boundaries()
{
	if (pos.x < -1010)
		pos.x += 2000;
	if (pos.z < -1010)
		pos.z += 2000;

	if (pos.x > 1010)
		pos.x -= 2000;
	if (pos.z > 1010)
		pos.z -= 2000;
}