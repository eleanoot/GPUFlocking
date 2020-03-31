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

	GetPhysicsObject()->SetInverseMass(1);
	GetPhysicsObject()->InitSphereInertia();
	GetPhysicsObject()->SetLinearVelocity(Vector3(rand() % 3 + 0.01 * 10, 0, rand() % 3 + 0.01 * 10));
}

void GPUBoid::OnSetup()
{
	// persistent pointers?
}

void GPUBoid::OnDraw()
{
	/*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, flockSSBO[bufferIndex]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, flockSSBO[bufferIndex ^ 1]);

	flockShader->Bind();
	flockShader->Execute(64, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFinish();
	flockShader->Unbind();

	bufferIndex ^= 1;*/
}