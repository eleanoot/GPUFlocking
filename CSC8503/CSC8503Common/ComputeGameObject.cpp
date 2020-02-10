#include "ComputeGameObject.h"

using namespace NCL;
using namespace NCL::CSC8503;

ComputeGameObject::ComputeGameObject(string name) : GameObject(name)
{
	positionShader = new OGLComputeShader("PositionComputeShader.glsl");
	glGenBuffers(1, &posSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Vector3), NULL, GL_STATIC_DRAW);
	

	// Colour changing
	/*colourShader = new OGLComputeShader("ColourComputeShader.glsl");

	glGenBuffers(1, &colourSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, colourSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, noOfColours * sizeof(Vector4), NULL, GL_STATIC_DRAW);*/
}

ComputeGameObject::~ComputeGameObject()
{
	delete positionShader;
}

void ComputeGameObject::OnSetup()
{
	Vector3 startPos = transform.GetWorldPosition();
	GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
	Vector3* ptr = (Vector3*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Vector3), bufMask);

	ptr[0] = startPos;

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ComputeGameObject::OnDraw()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
	positionShader->Bind();
	positionShader->Execute(128, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	

	Vector3* ptr = (Vector3*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Vector3), GL_MAP_READ_BIT);

	transform.SetWorldPosition(ptr[0]);

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	glFinish();
	positionShader->Unbind();

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