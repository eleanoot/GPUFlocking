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
	gridRowShader = new OGLComputeShader("GridRowCounts.glsl");

	flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	// bind counts, offsets, ranges, indexes to shader buffers 
	glGenBuffers(1, &countsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, countsBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * cellCount, 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	
	glGenBuffers(1, &offsetsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, offsetsBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * cellCount, 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	oPtr = (GLuint*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * cellCount, flags);

	glGenBuffers(1, &atomicOffsetsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomicOffsetsBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * cellCount, 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	aPtr = (GLuint*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * cellCount, flags);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * flockSize, 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

	glGenBuffers(1, &gridRowCountsBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gridRowCountsBuffer);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * cellCounts.x, 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

	// actual boid objects still created in flocking sim 


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

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, countsBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, offsetsBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, atomicOffsetsBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, indexBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, gridRowCountsBuffer);
	

	if (offsets) delete[] offsets;
	offsets = new GLuint[cellCount];

	if (ranges) delete[] ranges;
	ranges = new range[cellCount];
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

void FlockSystem::UpdatePartitionFlock(float dt)
{

	// bind buffers for position in and out 
	obstacles[0]->UpdateObstacle(dt);
	obPtr[0].centre = obstacles[0]->GetTransform().GetWorldPosition();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, flockSSBO[bufferIndex]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, flockSSBO[bufferIndex ^ 1]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, obstacleSSBO);


	// dispatch cell count shader- determine which cell each boid is in and fill counts buffer with how many boids are in there
	cellCountShader->Bind();
	glUniform1f(glGetUniformLocation(cellCountShader->GetProgramID(), "ratio"), cellRatio);
	glUniform1i(glGetUniformLocation(cellCountShader->GetProgramID(), "numBoids"), flockSize);
	glUniform1i(glGetUniformLocation(cellCountShader->GetProgramID(), "cellCount"), cellCount);
	glUniform2ui(glGetUniformLocation(cellCountShader->GetProgramID(), "cellCounts"), cellCounts.x, cellCounts.y);
	cellCountShader->Execute(flockSize / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	cellCountShader->Unbind();

	// get counts based on ranges 
	GLuint* counts = (GLuint*)ranges;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, countsBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * cellCount, counts);

	// CPU VERSION
	// form offset buffer from the counts- this will eventually tell indexing where to start for going through specific cells
	// based on how many boids are listed as being in each cell
	//GLuint rollingOffset = 0;
	//for (int i = 0; i < cellCount; ++i)
	//{
	//	offsets[i] = rollingOffset;
	//	rollingOffset += counts[i];
	//	// Update the actual offsets buffer by its persistent pointer so the shader gets this info
	//	oPtr[i] = offsets[i];
	//	aPtr[i] = offsets[i];
	//}

	// GPU VERSION
	// make sure the offsets end up in offset AND atomic offset buffers for Indexer!!
	gridRowShader->Bind();
	glUniform1i(glGetUniformLocation(gridRowShader->GetProgramID(), "cellCount"), cellCount);
	glUniform2ui(glGetUniformLocation(gridRowShader->GetProgramID(), "cellCounts"), cellCounts.x, cellCounts.y);
	gridRowShader->Execute(cellCounts.x, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	gridRowShader->Unbind();

	// dispatch index shader
	indexShader->Bind();
	glUniform1f(glGetUniformLocation(indexShader->GetProgramID(), "ratio"), cellRatio);
	glUniform1i(glGetUniformLocation(indexShader->GetProgramID(), "numBoids"), flockSize);
	glUniform1i(glGetUniformLocation(indexShader->GetProgramID(), "cellCount"), cellCount);
	glUniform2ui(glGetUniformLocation(indexShader->GetProgramID(), "cellCounts"), cellCounts.x, cellCounts.y);

	indexShader->Execute(flockSize / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	indexShader->Unbind();

	// dispatch boid rules shader- basic movement as before (currently based on GPUBoid.glsl)
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
	glUniform1f(glGetUniformLocation(flockShader->GetProgramID(), "ratio"), cellRatio);
	glUniform1i(glGetUniformLocation(flockShader->GetProgramID(), "numBoids"), flockSize);
	glUniform1i(glGetUniformLocation(flockShader->GetProgramID(), "cellCount"), cellCount);
	glUniform2ui(glGetUniformLocation(flockShader->GetProgramID(), "cellCounts"), cellCounts.x, cellCounts.y);

	flockShader->Execute(flockSize / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	flockShader->Unbind();

	bufferIndex ^= 1;

	for (int i = 0; i < gpuBoids.size(); i++)
	{
		if (bufferIndex == 0)
		{
			gpuBoids[i]->GetTransform().SetWorldPosition(fmPtrTwo[i].position);
			float theta = fmPtrTwo[i].angle;
			gpuBoids[i]->GetRenderObject()->GetTransform()->SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), theta));
		}

		else
		{
			gpuBoids[i]->GetTransform().SetWorldPosition(fmPtrOne[i].position);
			float theta = fmPtrOne[i].angle;
			gpuBoids[i]->GetRenderObject()->GetTransform()->SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), theta));
		}

	}

	// clear out the buffers from last time so they dont accumulate forever
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, countsBuffer);
	glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexBuffer);
	glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, offsetsBuffer);
	glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomicOffsetsBuffer);
	glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}


