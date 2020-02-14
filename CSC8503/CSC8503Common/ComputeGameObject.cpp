#include "ComputeGameObject.h"

using namespace NCL;
using namespace NCL::CSC8503;

ComputeGameObject::ComputeGameObject(string name) : GameObject(name)
{
	/* MOVE THIS SOMEWHERE ELSE BECAUSE WE DONT WANT BOID COMPUTE OBJECTS TO RUN IT EITHER WAY */
	// Persistent buffer creation
	//positionShader = new OGLComputeShader("DoubleBufferPositionComputeShader.glsl");
	//flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	//glGenBuffers(2, posSSBO);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO[0]);
	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(Vector3), NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	//// Persistent buffer mapping
	//posPtrBegin = (Vector3*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Vector3), flags);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO[1]);
	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(Vector3), NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	//posPtrSecond = (Vector3*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Vector3), flags);

	//bufferIndex = 0;

	// Basic movement
	/*positionShader = new OGLComputeShader("PositionComputeShader.glsl");
	glGenBuffers(1, &posSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Vector3), NULL, GL_STATIC_DRAW);*/
	

	// Colour changing
	/*colourShader = new OGLComputeShader("ColourComputeShader.glsl");

	glGenBuffers(1, &colourSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, colourSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, noOfColours * sizeof(Vector4), NULL, GL_STATIC_DRAW);*/
}

ComputeGameObject::~ComputeGameObject()
{
	/*delete positionShader;
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);*/
}

void ComputeGameObject::OnSetup()
{
	// Persistent buffer mapping
	//posPtrBegin = (Vector3*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Vector3), flags);
	//posPtrSecond = (Vector3*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Vector3), flags); // cant map a buffer again after mapping it 
	
	// Persistence position setting
	//Vector3 startPos = transform.GetWorldPosition();
	//*posPtrBegin = startPos;
	//*posPtrSecond = startPos;

	//transform.SetPositionPointer(posPtrBegin);

	// Basic movement
	//Vector3 startPos = transform.GetWorldPosition();
	/*Vector3 startPos = physicsObject->GetLinearVelocity();
	GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
	Vector3* ptr = (Vector3*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Vector3), bufMask);

	ptr[0] = startPos;

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);*/
}

void ComputeGameObject::OnDraw()
{
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO[bufferIndex]);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posSSBO[bufferIndex ^ 1]);

	//positionShader->Bind();
	//positionShader->Execute(1, 1, 1);
	//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	//glFinish();
	//positionShader->Unbind();

	//// edit the position shader to output to the buffer bound to index 1 instead like the opengl superbible example
	//// then switch the one the pointer points to

	//bufferIndex ^= 1;

	//if (bufferIndex == 0)
	//	transform.SetPositionPointer(posPtrBegin);
	//else
	//	transform.SetPositionPointer(posPtrSecond);

	

	// Basic movement
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
	//positionShader->Bind();
	//positionShader->Execute(128, 1, 1);
	//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	//

	//Vector3* ptr = (Vector3*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Vector3), GL_MAP_READ_BIT);

	////transform.SetWorldPosition(ptr[0]);
	//physicsObject->SetLinearVelocity(ptr[0]);

	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//glFinish();
	//positionShader->Unbind();

	// Colour changing
	/*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, colourSSBO);
	colourShader->Bind();
	colourShader->Execute(4, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFinish();

	colourShader->Unbind();*/
}

void ComputeGameObject::SendUniforms(OGLShader* s)
{
	glUniform1i(glGetUniformLocation(s->GetProgramID(), "colourIndex"), 1);

	colourCount++;
	if (colourCount >= noOfColours) colourCount = 0;
}