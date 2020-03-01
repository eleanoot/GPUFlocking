#include "GPUBoid.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../CSC8503Common/GameWorld.h"
using namespace NCL;
using namespace NCL::CSC8503;

GPUBoid::GPUBoid(float x, float z, OGLMesh* mesh, OGLShader* shader) : ComputeGameObject("GPU BOID", false)
{
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

void GPUBoid::OnSetup()
{
	// buffers etc
}

void GPUBoid::OnDraw()
{
	// running shader etc
}