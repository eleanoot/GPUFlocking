#include "CPUBoid.h"

using namespace NCL;
using namespace CSC8503;

CPUBoid::CPUBoid(float x, float z)
{
	accel = Vector3(0, 0, 0);
	vel = Vector3(rand() % 20 - 10, 0, rand() % 20 - 10);
	pos = Vector3(x, 0, z);
	maxSpeed = 20.5;
	maxForce = 0.5;
}

void CPUBoid::ApplyForce(Vector3 force)
{
	physicsObject->AddForce(force);
}

Vector3 CPUBoid::Separation(std::vector<CPUBoid*> boids)
{
	// Field of vision distance
	float sepDis = 20;
	Vector3 steer = Vector3(0, 0, 0);
	int neighbourCount = 0;

	for (int i = 0; i < boids.size(); i++)
	{
		// Calculate distance from this boid to the one we're looking at
		float distance = (transform.GetWorldPosition() - boids[i]->GetTransform().GetWorldPosition()).Length();

		// If this is a boid and it's too close, move away from it
		if (distance > 0 && (distance < sepDis))
		{
			Vector3 diff = Vector3(0, 0, 0);
			diff = transform.GetWorldPosition() - boids[i]->GetTransform().GetWorldPosition();
			diff.Normalise();
			diff /= distance; // Weight by the distance
			steer += diff;
			neighbourCount++;
		}

		// Add the average difference of location to acceleration 
		if (neighbourCount > 0)
			steer /= (float)neighbourCount;

		if (steer.Length() > 0)
		{
			// Steering = desired - velocity
			steer.Normalise();
			steer *= maxSpeed;
			steer -= physicsObject->GetLinearVelocity();
		}

		return steer;
	}
}

Vector3 CPUBoid::Alignment(std::vector<CPUBoid*> boids)
{
	float neighDis = 50;

	Vector3 sum = Vector3(0, 0, 0);
	int neighbourCount = 0;

	for (int i = 0; i < boids.size(); i++)
	{
		float distance = (transform.GetWorldPosition() - boids[i]->GetTransform().GetWorldPosition()).Length();

		if (distance > 0 && distance < neighDis)
		{
			sum += boids[i]->GetPhysicsObject()->GetLinearVelocity();
			neighbourCount++;
		}
	}

	if (neighbourCount > 0)
	{
		sum /= (float)neighbourCount;
		sum.Normalise();
		sum *= maxSpeed;

		Vector3 steer;
		steer = sum - physicsObject->GetLinearVelocity();
		return steer;
	}
	else
		return Vector3(0, 0, 0);
}

Vector3 CPUBoid::Cohesion(std::vector<CPUBoid*> boids)
{
	float neighDis = 50;
	Vector3 sum(0, 0, 0);
	int neighbourCount = 0;

	for (int i = 0; i < boids.size(); i++)
	{
		float distance = (transform.GetWorldPosition() - boids[i]->GetTransform().GetWorldPosition()).Length();
		if (distance > 0 && distance < neighDis)
		{
			sum += boids[i]->GetTransform().GetWorldPosition();
			neighbourCount++;
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
	accel = desired - physicsObject->GetLinearVelocity();
	return accel;
}

void CPUBoid::Update(std::vector<CPUBoid*> boids)
{
	Vector3 sep = Separation(boids);
	Vector3 align = Alignment(boids);
	Vector3 cohesion = Cohesion(boids);

	// Random weighing right now
	sep *= 1.5;
	align *= 1.0;
	cohesion *= 1.0;

	// Add the force vectors to acceleration 
	ApplyForce(sep);
	ApplyForce(align);
	ApplyForce(cohesion);
}