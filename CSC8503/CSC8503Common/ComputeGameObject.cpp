#include "ComputeGameObject.h"

using namespace NCL;
using namespace NCL::CSC8503;

ComputeGameObject::ComputeGameObject(string name) : GameObject(name)
{
	// Persistent buffer creation
	positionShader = new OGLComputeShader("PositionComputeShader.glsl");
	glGenBuffers(1, &posSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(Vector3), NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

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
	delete positionShader;
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ComputeGameObject::OnSetup()
{
	// Persisten buffer mapping
	GLint flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	posPtr = (Vector3*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Vector3), flags);
	// Persistence position setting
	Vector3 startPos = transform.GetWorldPosition();
	posPtr[0] = startPos;

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