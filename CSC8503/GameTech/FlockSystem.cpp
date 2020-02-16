#include "FlockSystem.h"
using namespace NCL;
using namespace NCL::CSC8503;

FlockSystem::FlockSystem(int noOfBoids, GameWorld* world, OGLMesh* mesh, OGLShader* shader)
{
	boidMesh = mesh;

	int colourCount = 0;
	// Randomly create a number of boids with a position, velocity, colour, rotation(?)
	for (int i = 0; i < noOfBoids; i++)
	{
		CPUBoid* boid = new CPUBoid("BOID", false);

		SphereVolume* volume = new SphereVolume(1.0f);
		boid->SetBoundingVolume((CollisionVolume*)volume);

		boid->GetTransform().SetWorldScale(Vector3(1, 1, 1));

		boid->GetTransform().SetWorldPosition(Vector3(rand() % 80, 2, rand() % 80));

		boid->SetRenderObject(new RenderObject(&boid->GetTransform(), boidMesh, nullptr, shader));
		boid->SetPhysicsObject(new PhysicsObject(&boid->GetTransform(), boid->GetBoundingVolume()));

		boid->GetPhysicsObject()->SetInverseMass(1);
		boid->GetPhysicsObject()->InitSphereInertia();

		
		boid->GetPhysicsObject()->SetLinearVelocity(Vector3(rand() % 20, 0, rand() % 20));
		boid->GetRenderObject()->SetColour(colours[colourCount % 3]);
		colourCount++;

		world->AddGameObject(boid);
		allBoids.push_back(boid);
	}
}

FlockSystem::~FlockSystem()
{
	allBoids.clear();
}

void FlockSystem::UpdateFlock(float dt)
{
	FindNeighbours();
	//Vector3 dir = Vector3(0, 0, 1);
	Vector3 dir;
	for (int i = 0; i < allBoids.size(); i++)
	{
		//allBoids[i]->GetTransform().SetWorldPosition(allBoids[i]->GetTransform().GetWorldPosition() + dir);
		dir += Separation(allBoids[i]);
		dir += Alignment(allBoids[i]);
		//dir += Cohesion(allBoids[i]);

		//allBoids[i]->GetTransform().SetWorldPosition(allBoids[i]->GetTransform().GetWorldPosition() + dir);
		allBoids[i]->GetPhysicsObject()->SetLinearVelocity(dir);
		allBoids[i]->GetTransform().SetWorldPosition(dir + allBoids[i]->GetTransform().GetWorldPosition());
		// position += velocity * dt
	}
	
}

void FlockSystem::FindNeighbours()
{
	for (auto it = allBoids.begin(); it != allBoids.end() - 1; it++)
	{
		CPUBoid* currentBoid = *it;
		for (auto n = allBoids.begin() + 1; n != allBoids.end(); n++)
		{
			CPUBoid* testBoid = *n;
			float xDis = abs(currentBoid->GetTransform().GetWorldPosition().x - testBoid->GetTransform().GetWorldPosition().x);
			float zDis = abs(currentBoid->GetTransform().GetWorldPosition().z - testBoid->GetTransform().GetWorldPosition().z);
			if (xDis <= NEIGHBOUR_RADIUS && zDis <= NEIGHBOUR_RADIUS)
			{
				currentBoid->AddNeighbour(testBoid);
				testBoid->AddNeighbour(currentBoid);
			}
		}
	}
}

Vector3 FlockSystem::Separation(CPUBoid* b)
{
	Vector3 dir;
	std::vector<CPUBoid*>::const_iterator first;
	std::vector<CPUBoid*>::const_iterator last;
	b->GetNeighourIterators(first, last);
	for (auto it = first; it != last; it++)
	{
		Vector3 dis = b->GetTransform().GetWorldPosition() - (*it)->GetTransform().GetWorldPosition();
		float distance = dis.Length();

		if (distance > FLOCK_SEPARATION)
			continue;

		float strength = 1.0f - (distance / FLOCK_SEPARATION);

		dir += (b->GetTransform().GetWorldPosition() - (*it)->GetTransform().GetWorldPosition()).Normalised() * strength;
	}

	return dir.Normalised();
}

Vector3 FlockSystem::Alignment(CPUBoid* b)
{
	Vector3 avgVelocity = b->GetPhysicsObject()->GetLinearVelocity();
	std::vector<CPUBoid*>::const_iterator first;
	std::vector<CPUBoid*>::const_iterator last;
	b->GetNeighourIterators(first, last);
	int neighbourCount = 0;
	for (auto it = first; it != last; it++)
	{
		avgVelocity += (*it)->GetPhysicsObject()->GetLinearVelocity();
		neighbourCount++;
	}

	avgVelocity /= (neighbourCount - 1);


	return ((avgVelocity - b->GetPhysicsObject()->GetLinearVelocity()) / 8).Normalised();
}

Vector3 FlockSystem::Cohesion(CPUBoid* b)
{
	return Vector3(0, 0, 0);
}