#include "UniformsRenderObject.h"

using namespace NCL::CSC8503;
using namespace NCL;

UniformsRenderObject::UniformsRenderObject(Transform* parentTransform, MeshGeometry* mesh, TextureBase* tex, ShaderBase* shader)
	: RenderObject(parentTransform, mesh, tex, shader)
{
	
}

UniformsRenderObject::~UniformsRenderObject()
{

}

void UniformsRenderObject::SendUniforms()
{
	
}