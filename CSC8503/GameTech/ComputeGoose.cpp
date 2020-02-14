#include "ComputeGoose.h"
using namespace NCL;
using namespace NCL::CSC8503;
ComputeGoose::ComputeGoose(string name) : ComputeGameObject(name)
{
	// Persistent buffer creation
	positionShader = new OGLComputeShader("DoubleBufferPositionComputeShader.glsl");
	flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	glGenBuffers(2, posSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO[0]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(Vector3), NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	// Persistent buffer mapping
	posPtrBegin = (Vector3*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Vector3), flags);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO[1]);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(Vector3), NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	posPtrSecond = (Vector3*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Vector3), flags);

	bufferIndex = 0;
}

ComputeGoose::~ComputeGoose()
{
	delete positionShader;
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ComputeGoose::OnSetup()
{

	// Persistence position setting
	Vector3 startPos = transform.GetWorldPosition();
	*posPtrBegin = startPos;
	*posPtrSecond = startPos;

	transform.SetPositionPointer(posPtrBegin);
}

void ComputeGoose::OnDraw()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO[bufferIndex]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posSSBO[bufferIndex ^ 1]);

	positionShader->Bind();
	positionShader->Execute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFinish();
	positionShader->Unbind();

	bufferIndex ^= 1;

	if (bufferIndex == 0)
		transform.SetPositionPointer(posPtrBegin);
	else
		transform.SetPositionPointer(posPtrSecond);
}

void ComputeGoose::SendUniforms(OGLShader* s)
{
	glUniform1i(glGetUniformLocation(s->GetProgramID(), "colourIndex"), 1);

	colourCount++;
	if (colourCount >= noOfColours) colourCount = 0;
}