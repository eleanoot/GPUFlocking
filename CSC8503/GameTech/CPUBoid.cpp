#include "CPUBoid.h"

using namespace NCL;
using namespace CSC8503;

CPUBoid::CPUBoid(float x, float z, OGLMesh* mesh, OGLShader* shader) : GameObject("BOID", false)
{
	accel = Vector3(0, 0, 0);
	vel = Vector3(rand() % 3, 0, rand() % 3);
	pos = Vector3(x, 0, z);

	SphereVolume* volume = new SphereVolume(1.0f);
	SetBoundingVolume((CollisionVolume*)volume);

	transform.SetWorldScale(Vector3(10, 10, 10));

	transform.SetWorldPosition(Vector3(x, 0, z));

	SetRenderObject(new RenderObject(&transform, mesh, nullptr, shader));
	SetPhysicsObject(new PhysicsObject(&transform, GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(1);
	GetPhysicsObject()->InitSphereInertia();

	GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	GetPhysicsObject()->SetLinearVelocity(Vector3(rand() % 3, 0, rand() % 3));
}

void CPUBoid::ApplyForce(Vector3 force)
{
//	physicsObject->AddForce(force);
	accel += force;
}

Vector3 CPUBoid::Separation(std::vector<CPUBoid*> boids)
{
	// Field of vision distance
	float sepDis = 10;
	Vector3 steer = Vector3(0, 0, 0);
	int neighbourCount = 0;

	for (int i = 0; i < boids.size(); i++)
	{
		if (boids[i] != this)
		{
			// Calculate distance from this boid to the one we're looking at
			//float distance = (transform.GetWorldPosition() - boids[i]->GetTransform().GetWorldPosition()).Length();
			float distance = (pos - boids[i]->pos).Length();

			// If this is a boid and it's too close, move away from it
			if (distance > 0 && (distance < sepDis))
			{
				Vector3 diff = Vector3(0, 0, 0);
				//diff = transform.GetWorldPosition() - boids[i]->GetTransform().GetWorldPosition();
				diff = pos - boids[i]->pos;
				diff.Normalise();
				diff /= distance; // Weight by the distance
				steer += diff;
				neighbourCount++;
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
		//steer -= physicsObject->GetLinearVelocity();
		steer -= vel;
		//steer.Limit(maxForce);
	}

	return steer;
}

Vector3 CPUBoid::Alignment(std::vector<CPUBoid*> boids)
{
	float neighDis = 8;

	Vector3 sum = Vector3(0, 0, 0);
	int neighbourCount = 0;

	for (int i = 0; i < boids.size(); i++)
	{
		if (boids[i] != this)
		{
			//float distance = (transform.GetWorldPosition() - boids[i]->GetTransform().GetWorldPosition()).Length();
			float distance = (pos - boids[i]->pos).Length();

			if (distance > 0 && distance < neighDis)
			{
				//sum += boids[i]->GetPhysicsObject()->GetLinearVelocity();
				sum += boids[i]->vel;
				neighbourCount++;
			}
		}
		
	}

	if (neighbourCount > 0)
	{
		sum /= (float)neighbourCount;
		sum.Normalise();

		Vector3 steer;
		//steer = sum - physicsObject->GetLinearVelocity();
		steer = sum - vel;
		return steer;
	}
	else
		return Vector3(0, 0, 0);
}

Vector3 CPUBoid::Cohesion(std::vector<CPUBoid*> boids)
{
	float neighDis = 8;
	Vector3 sum(0, 0, 0);
	int neighbourCount = 0;

	for (int i = 0; i < boids.size(); i++)
	{
		if (boids[i] != this)
		{
		//	float distance = (transform.GetWorldPosition() - boids[i]->GetTransform().GetWorldPosition()).Length();
			float distance = (pos - boids[i]->pos).Length();
			if (distance > 0 && distance < neighDis)
			{
				//sum += boids[i]->GetTransform().GetWorldPosition();
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
	//physicsObject->AddForce(desired - physicsObject->GetLinearVelocity());
	accel = desired - vel;
	//return physicsObject->GetForce();
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

	accel *= 0.4;
	vel += accel;
	pos += vel;
	accel *= 0;
}

float CPUBoid::Angle(Vector3 v)
{
	// Adjusting the pitch of the boid
	return (float)(atan2(v.x, -v.z) * 180 / 3.14);
}