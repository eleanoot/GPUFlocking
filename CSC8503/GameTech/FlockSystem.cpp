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

	glGenBuffers(1, &obstacleSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, obstacleSSBO);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(obstacle) * obstacleData.size(), &obstacleData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	obPtr = (obstacle*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(obstacle) * obstacles.size(), flags);

}

void FlockSystem::InitInstanceFlock(OGLMesh* m, RenderObject* r)
{
	boidMesh = m;
	flock_member fm;
	// set up the flock member data
	for (int i = 0; i < FLOCK_SIZE; i++)
	{
		fm.position = Vector3(rand() % 1000, 0, rand() % 1000);
		fm.velocity = Vector3(rand() % 6 + (-3) + 0.01 * 10, 0, rand() % 6 + (-3) + 0.01 * 10);
		fm.accel = Vector3(0, 0, 0);
		int groupNo = rand() % 3 + 1;
		fm.groupNo = groupNo;
		gpuData.emplace_back(fm);
	}

	flockShader = new OGLComputeShader("InstanceBoid.glsl");
	flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	glGenBuffers(2, flockSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, flockSSBO[0]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member) * FLOCK_SIZE, &gpuData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	fmPtrOne = (flock_member*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(flock_member) * FLOCK_SIZE, flags);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, flockSSBO[1]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member) * FLOCK_SIZE, &gpuData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	fmPtrTwo = (flock_member*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(flock_member) * FLOCK_SIZE, flags);

	bufferIndex = 0;
	r->SetSSBO(flockSSBO[0], flockSSBO[1]);

	glGenBuffers(1, &obstacleSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, obstacleSSBO);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(obstacle) * obstacleData.size(), &obstacleData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	obPtr = (obstacle*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(obstacle) * obstacles.size(), flags);
}

void FlockSystem::AddBoid(GPUBoid* b)
{
	gpuBoids.push_back(b);
	flock_member fm;
	fm.position = b->GetTransform().GetWorldPosition();
	fm.velocity = b->GetPhysicsObject()->GetLinearVelocity(); 
	fm.accel = Vector3(0, 0, 0);
	fm.angle = 0;
	int groupNo = rand() % 3 + 1;
	fm.groupNo = groupNo;
	gpuData.push_back(fm);

	Vector4 colour = Vector4(1, 1, 1, 1);
	switch (groupNo)
	{
	case 1: colour = Vector4(1, 0, 0, 1); break;
	case 2: colour = Vector4(0, 1, 0, 1); break;
	case 3: colour = Vector4(0, 0, 1, 1); break;
	default: break;
	}

	b->GetRenderObject()->SetColour(colour);
}

void FlockSystem::UpdateFlock(float dt)
{
	obstacles[0]->UpdateObstacle(dt);
	for (int i = 0; i < allBoids.size(); i++)
	{
		allBoids[i]->Update(allBoids, obstacles);
	}
}


void FlockSystem::UpdateGPUFlock(float dt)
{
	obstacles[0]->UpdateObstacle(dt);
	obPtr[0].centre = obstacles[0]->GetTransform().GetWorldPosition();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, flockSSBO[bufferIndex]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, flockSSBO[bufferIndex ^ 1]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, obstacleSSBO);

	flockShader->Bind();
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "sepDis"), 20);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "alignDis"), 40);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "cohDis"), 15);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "sepWeight"), 150);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "alignWeight"), 50);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "avoidWeight"), 150);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "cohWeight"), 25);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxSpeed"), 200);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxForce"), 70);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "dt"), dt);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxSeeAhead"), 200);
	glUniform1i(glGetUniformLocation(flockShader->GetProgramID(), "noOfObstacles"), obstacleData.size());

	flockShader->Execute(FLOCK_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFinish();
	flockShader->Unbind();

	bufferIndex ^= 1;

	for (int i = 0; i < gpuBoids.size(); i++)
	{
		if (bufferIndex == 0)
		{
			gpuBoids[i]->GetTransform().SetWorldPosition(fmPtrTwo[i].position);
			float theta = fmPtrTwo[i].angle;
			gpuBoids[i]->GetRenderObject()->GetTransform()->SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0,1,0), theta));
		}
			
		else
		{
			gpuBoids[i]->GetTransform().SetWorldPosition(fmPtrOne[i].position);
			float theta = fmPtrOne[i].angle;
			gpuBoids[i]->GetRenderObject()->GetTransform()->SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), theta));
		}
			
	}

}



void FlockSystem::UpdateInstanceFlock(float dt)
{
	obstacles[0]->UpdateObstacle(dt);
	obPtr[0].centre = obstacles[0]->GetTransform().GetWorldPosition();
	// Execute the compute shader to get the positions
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, flockSSBO[bufferIndex]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, flockSSBO[bufferIndex ^ 1]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, obstacleSSBO);

	flockShader->Bind();
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "sepDis"), 20);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "alignDis"), 40);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "cohDis"), 15);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "sepWeight"), 150);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "alignWeight"), 50);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "cohWeight"), 25);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "avoidWeight"), 150);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxSpeed"), 200);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxForce"), 70);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "dt"), dt);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxSeeAhead"), 200);
	glUniform1i(glGetUniformLocation(flockShader->GetProgramID(), "noOfObstacles"), obstacleData.size());
	flockShader->Execute(FLOCK_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFinish();
	flockShader->Unbind();

	bufferIndex ^= 1;

}




