#include "ComputeGameObject.h"

using namespace NCL;
using namespace NCL::CSC8503;

ComputeGameObject::ComputeGameObject(string name) : GameObject(name)
{
	colourShader = new OGLComputeShader("ColourComputeShader.glsl");

	glGenBuffers(1, &colourSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, colourSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, noOfColours * sizeof(Vector4), NULL, GL_STATIC_DRAW);
}

ComputeGameObject::~ComputeGameObject()
{
	delete colourShader;
}

void ComputeGameObject::OnDraw()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, colourSSBO);
	colourShader->Bind();
	colourShader->Execute(4, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFinish();

	colourShader->Unbind();
}

void ComputeGameObject::SendUniforms()
{
	glUniform1i(glGetUniformLocation(colourShader->GetProgramID(), "colourIndex"), colourCount);

	colourCount++;
	if (colourCount >= noOfColours) colourCount = 0;
}