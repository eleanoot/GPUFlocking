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
	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member) * flockSize, &gpuData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member) * flockSize, &gpuData[0], GL_STATIC_DRAW);

	/*glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, flockSSBO, 0, sizeof(flock_member) * flockSize);
	flock_member* fm = (flock_member*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(flock_member) * flockSize, GL_MAP_READ_BIT);
	std::cout << fm->position << "," << fm->velocity << std::endl;*/

	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
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

void FlockSystem::UpdateGPUFlock(float dt)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, flockSSBO);

	flockShader->Bind();
	flockShader->Execute(128, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFinish();
	flockShader->Unbind();

	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, flockSSBO, 0, sizeof(flock_member) * gpuBoids.size());
	flock_member* fm = (flock_member*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(flock_member) * gpuBoids.size(), GL_MAP_READ_BIT);

	for (int i = 0; i < gpuBoids.size(); i++)
	{
		gpuData[i] = fm[i];
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	std::cout << gpuData[0].position << "," << gpuData[0].velocity << std::endl;
}



