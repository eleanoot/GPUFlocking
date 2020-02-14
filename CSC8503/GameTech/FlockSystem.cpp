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
		GameObject* boid = new GameObject();

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
	}
}

FlockSystem::~FlockSystem()
{
	allBoids.clear();
}