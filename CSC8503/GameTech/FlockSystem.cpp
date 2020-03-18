#include "FlockSystem.h"
#include "../GameTech/CPUBoid.h"
using namespace NCL;
using namespace NCL::CSC8503;

#define FLOCK_SIZE 256

FlockSystem::FlockSystem()
{
	
}

FlockSystem::~FlockSystem()
{
	allBoids.clear();
	gpuBoids.clear();
	delete flockShader;
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void FlockSystem::InitGPU()
{
	flockShader = new OGLComputeShader("GPUBoid.glsl");
	flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	glGenBuffers(2, flockSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, flockSSBO[0]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member) * FLOCK_SIZE, &gpuData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	fmPtrOne = (flock_member*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(flock_member) * FLOCK_SIZE, flags);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, flockSSBO[1]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member) * FLOCK_SIZE, &gpuData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	fmPtrTwo = (flock_member*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(flock_member) * FLOCK_SIZE, flags);

	bufferIndex = 0;
}

void FlockSystem::AddBoid(GPUBoid* b)
{
	gpuBoids.push_back(b);
	flock_member fm;
	fm.position = b->GetTransform().GetWorldPosition();
	fm.velocity = b->GetPhysicsObject()->GetLinearVelocity(); 
	fm.accel = Vector3(0, 0, 0);
	gpuData.push_back(fm);
}

void FlockSystem::UpdateFlock(float dt)
{
	for (int i = 0; i < allBoids.size(); i++)
	{
		allBoids[i]->Update(allBoids);
	}
}

float Angle(Vector3 v)
{
	// Adjusting the pitch of the boid
	return (float)(atan2(v.x, v.z) * 180 / 3.14);
}


void FlockSystem::UpdateGPUFlock(float dt)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, flockSSBO[bufferIndex]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, flockSSBO[bufferIndex ^ 1]);

	flockShader->Bind();
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "sepDis"), 60);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "alignDis"), 70);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "cohDis"), 25);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "sepWeight"), 300);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "alignWeight"), 200);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "cohWeight"), 25);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxSpeed"), 30.5);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxForce"), 5);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "dt"), dt);
	flockShader->Execute(256, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFinish();
	flockShader->Unbind();

	bufferIndex ^= 1;

	for (int i = 0; i < gpuBoids.size(); i++)
	{
		if (bufferIndex == 0)
		{
			gpuBoids[i]->GetTransform().SetWorldPosition(fmPtrTwo[i].position);
			float theta = Angle(fmPtrTwo[i].velocity);
			gpuBoids[i]->GetRenderObject()->GetTransform()->SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0,1,0), theta));
		}
			
		else
		{
			gpuBoids[i]->GetTransform().SetWorldPosition(fmPtrOne[i].position);
			float theta = Angle(fmPtrOne[i].velocity);
			gpuBoids[i]->GetRenderObject()->GetTransform()->SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), theta));
		}
			
	}

}

void FlockSystem::InitInstanceFlock(OGLMesh* m, RenderObject* r)
{
	boidMesh = m;
	flock_member fm;
	// set up the flock member data
	for (int i = 0; i < FLOCK_SIZE; i++)
	{
		fm.position = Vector3(rand() % 200, 0, rand() % 200);
		fm.velocity = Vector3(rand() % 3 + 0.01 * 10, 0, rand() % 3 + 0.01 * 10);
		fm.accel = Vector3(0, 0, 0);
		gpuData.emplace_back(fm);
	}

	InitGPU();
	r->SetSSBO(flockSSBO[0], flockSSBO[1]);
}

void FlockSystem::UpdateInstanceFlock(float dt)
{
	// Execute the compute shader to get the positions
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, flockSSBO[bufferIndex]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, flockSSBO[bufferIndex ^ 1]);

	flockShader->Bind();
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "sepDis"), 60);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "alignDis"), 30);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "cohDis"), 10);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "sepWeight"), 200);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "alignWeight"), 200);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "cohWeight"), 25);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxSpeed"), 30.5);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxForce"), 5);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "dt"), dt);
	flockShader->Execute(256, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFinish();
	flockShader->Unbind();

	bufferIndex ^= 1;

	//std::cout << fmPtrOne[15].velocity << std::endl;
}




