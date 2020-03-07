#include "FlockSystem.h"
#include "../GameTech/CPUBoid.h"
using namespace NCL;
using namespace NCL::CSC8503;

FlockSystem::FlockSystem()
{
	
}

FlockSystem::~FlockSystem()
{
	allBoids.clear();
	delete flockShader;
}

void FlockSystem::InitGPU()
{
	flockShader = new OGLComputeShader("GPUBoid.glsl");
	flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	glGenBuffers(1, &flockSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, flockSSBO);
	int flockSize = gpuBoids.size();
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member) * flockSize, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
}

void FlockSystem::AddBoid(GPUBoid* b)
{
	gpuBoids.push_back(b);
	flock_member fm;
	fm.position = b->GetTransform().GetWorldPosition();
	fm.velocity = b->GetPhysicsObject()->GetLinearVelocity();
	gpuData.push_back(fm);
}

void FlockSystem::UpdateFlock(float dt)
{
	for (int i = 0; i < allBoids.size(); i++)
	{
		allBoids[i]->Update(allBoids);
	}
}



