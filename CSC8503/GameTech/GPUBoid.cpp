#include "GPUBoid.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../CSC8503Common/GameWorld.h"
using namespace NCL;
using namespace NCL::CSC8503;

GPUBoid::GPUBoid(float x, float z, OGLMesh* mesh, OGLShader* shader) : ComputeGameObject("GPU BOID", false)
{
	// Goose Mesh Creation
	flock_member fm;
	fm.position = Vector3(x, 0, z);
	fm.velocity = Vector3(rand() % 3, 0, rand() % 3);

	SphereVolume* volume = new SphereVolume(1.0f);
	SetBoundingVolume((CollisionVolume*)volume);

	transform.SetWorldScale(Vector3(10, 10, 10));

	transform.SetWorldPosition(Vector3(x, 0, z));

	SetRenderObject(new RenderObject(&transform, mesh, nullptr, shader));
	SetPhysicsObject(new PhysicsObject(&transform, GetBoundingVolume()));

	renderObject->SetColour(Vector4(rand() % 2, rand() % 2, rand() % 2, 1));

	GetPhysicsObject()->SetInverseMass(1);
	GetPhysicsObject()->InitSphereInertia();
	GetPhysicsObject()->SetLinearVelocity(fm.velocity);

	// Persistent buffers creation
	//flockShader = new OGLComputeShader("FlockingCompute.glsl");
	//flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	//glGenBuffers(2, flockSSBO);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, flockSSBO[0]);
	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member), NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

	////flockPtrFirst = (flock_member)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(flock_member), flags);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, flockSSBO[1]);
	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member), NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

	//bufferIndex = 0;

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