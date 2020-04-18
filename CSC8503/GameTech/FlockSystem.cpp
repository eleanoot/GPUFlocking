#include "FlockSystem.h"
#include "../GameTech/CPUBoid.h"
#include <algorithm>
using namespace NCL;
using namespace NCL::CSC8503;

#define WORK_GROUP_SIZE 128

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
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member) * flockSize, &gpuData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	fmPtrOne = (flock_member*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(flock_member) * flockSize, flags);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, flockSSBO[1]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member) * flockSize, &gpuData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	fmPtrTwo = (flock_member*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(flock_member) * flockSize, flags);

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
	for (int i = 0; i < flockSize; i++)
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
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member) * flockSize, &gpuData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	fmPtrOne = (flock_member*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(flock_member) * flockSize, flags);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, flockSSBO[1]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member) * flockSize, &gpuData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	fmPtrTwo = (flock_member*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(flock_member) * flockSize, flags);

	bufferIndex = 0;
	r->SetSSBO(flockSSBO[0], flockSSBO[1]);

	glGenBuffers(1, &obstacleSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, obstacleSSBO);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(obstacle) * obstacleData.size(), &obstacleData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	obPtr = (obstacle*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(obstacle) * obstacles.size(), flags);

	r->SetInstances(flockSize);
}

void FlockSystem::InitPartitionFlock()
{
	// init boid distances, cell size, ratio 
	sepDis = 20;
	alignDis = 40;
	cohDis = 15;

	cellSize = max(max(sepDis, alignDis), cohDis);
	cellRatio = 1.0f / cellSize;

	Vector2 worldBounds = Vector2(1000, 1000);
	cellCounts = Vector2(ceil(worldBounds.x / cellSize), ceil(worldBounds.y / cellSize));
	cellCount = cellCounts.x * cellCounts.y;

	// shader setup
	flockShader = new OGLComputeShader("PartitionBoid.glsl");
	cellCountShader = new OGLComputeShader("CellCounts.glsl");
	indexShader = new OGLComputeShader("Indexer.glsl");

	// bind counts, offsets, ranges, indexes to shader buffers 
	glGenBuffers(1, &countsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, countsBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * cellCount, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	
	glGenBuffers(1, &offsetsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, offsetsBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * cellCount, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

	glGenBuffers(1, &rangesBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, rangesBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(range) * cellCount, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexBuffer);

	// create the boid objects - done in FlockingSim still 
	flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	glGenBuffers(2, flockSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, flockSSBO[0]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member) * flockSize, &gpuData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	fmPtrOne = (flock_member*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(flock_member) * flockSize, flags);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, flockSSBO[1]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(flock_member) * flockSize, &gpuData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	fmPtrTwo = (flock_member*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(flock_member) * flockSize, flags);

	bufferIndex = 0;

	glGenBuffers(1, &obstacleSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, obstacleSSBO);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(obstacle) * obstacleData.size(), &obstacleData[0], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	obPtr = (obstacle*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(obstacle) * obstacles.size(), flags);


	// allocate cell buffers
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
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "avoidWeight"), 100);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "cohWeight"), 25);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxSpeed"), 200);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxForce"), 70);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "dt"), dt);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxSeeAhead"), 200);
	glUniform1i(glGetUniformLocation(flockShader->GetProgramID(), "noOfObstacles"), obstacleData.size());

	flockShader->Execute(flockSize / WORK_GROUP_SIZE, 1, 1);
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
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "avoidWeight"), 100);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxSpeed"), 200);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxForce"), 150);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "dt"), dt);
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "maxSeeAhead"), 200);
	glUniform1i(glGetUniformLocation(flockShader->GetProgramID(), "noOfObstacles"), obstacleData.size());
	flockShader->Execute(flockSize / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFinish();
	flockShader->Unbind();

	bufferIndex ^= 1;

}




