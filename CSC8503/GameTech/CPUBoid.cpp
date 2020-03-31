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

	GetPhysicsObject()->SetInverseMass(1);
	GetPhysicsObject()->InitSphereInertia();

	groupNo = rand() % 3 + 1;
	Vector4 colour = Vector4(1,1,1,1);
	switch (groupNo)
	{
	case 1:
		colour = Vector4(1, 0, 0, 1);
		break;
	case 2:
		colour = Vector4(0, 1, 0, 1);
		break;
	case 3:
		colour = Vector4(0, 0, 1, 1);
		break;
	default:
		break;
	}
	renderObject->SetColour(colour);
}

void CPUBoid::ApplyForce(Vector3 force)
{
	accel += force;
}

Vector3 CPUBoid::Separation(std::vector<CPUBoid*> boids)
{
	// Field of vision distance
	float sepDis = 60;
	float diffGroupDis = 90;
	Vector3 steer = Vector3(0, 0, 0);
	int neighbourCount = 0;

	for (int i = 0; i < boids.size(); i++)
	{
		if (boids[i] != this)
		{
			// Calculate distance from this boid to the one we're looking at
			float distance = (pos - boids[i]->pos).Length();
			// Boids will want to move further away from boids not in their group
			float dis = groupNo == boids[i]->groupNo ? sepDis : diffGroupDis;

			// If this is a boid and it's too close, move away from it
			if (distance > 0 && (distance < dis))
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

			if (distance > 0 && distance < neighDis)
			{
				sum += boids[i]->pos;

				if (groupNo == boids[i]->groupNo)
					sum *= 3.5;
				
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

Vector3 CPUBoid::Avoidance(std::vector<Obstacle*> obstacles)
{
	Vector3 sum(0, 0, 0);

	// ahead = position + normalize(velocity) * MAX_SEE_AHEAD
	// calculate the ahead vector
	Vector3 tempVel = vel.Normalised();
	Vector3 ahead = pos + tempVel * MAX_SEE_AHEAD;
	// calculate the ahead2 vector 
	Vector3 ahead2 = pos + tempVel * MAX_SEE_AHEAD * 0.5;

	// find the most threatening obstacle
	GameObject* mostThreateningObstacle = nullptr;
	for (int i = 0; i < obstacles.size(); i++)
	{
		GameObject* nextOb = obstacles[i]; 
		bool collision = LineCircleIntersect(ahead, ahead2, nextOb);
		Vector3 obDis = pos - nextOb->GetTransform().GetWorldPosition();
		Vector3 threatDis = Vector3(0,0,0);
		if (mostThreateningObstacle != nullptr)
			threatDis = pos - mostThreateningObstacle->GetTransform().GetWorldPosition();
		if (collision && (mostThreateningObstacle == nullptr || obDis.Length() < threatDis.Length()))
			mostThreateningObstacle = nextOb;
	}

	// calculate the avoidance force
	if (mostThreateningObstacle != nullptr)
	{
		sum = ahead - mostThreateningObstacle->GetTransform().GetWorldPosition();

		sum.Normalise();
		sum.Limit(maxForce);
	}

	return sum;
}

bool CPUBoid::LineCircleIntersect(Vector3 ahead, Vector3 ahead2, GameObject* obstacle)
{
	Vector3 dis = obstacle->GetTransform().GetWorldPosition() - ahead;
	Vector3 dis2 = obstacle->GetTransform().GetWorldPosition() - ahead2;
	Vector3 posDis = obstacle->GetTransform().GetWorldPosition() - pos;
	return dis.Length() <= obstacle->GetTransform().GetLocalScale().x
		|| dis2.Length() <= obstacle->GetTransform().GetLocalScale().x
		|| posDis.Length() <= obstacle->GetTransform().GetLocalScale().x;
}

void CPUBoid::Update(std::vector<CPUBoid*> boids, std::vector<Obstacle*> obstacles)
{
	Vector3 sep = Separation(boids);
	Vector3 align = Alignment(boids);
	Vector3 cohesion = Cohesion(boids);
	Vector3 avoidance = Avoidance(obstacles);

	// Random weighing right now
	sep *= 2.5;
	align *= 1.0;
	cohesion *= 1.0;
	avoidance *= 2.5;

	// Add the force vectors to acceleration 
	ApplyForce(sep);
	ApplyForce(align);
	ApplyForce(cohesion);
	ApplyForce(avoidance);

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
	v.Normalise();
	// Adjusting the pitch of the boid
	return (float)(atan2(-v.x, -v.z) * 180 / 3.14);
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