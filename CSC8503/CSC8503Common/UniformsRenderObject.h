#pragma once
#include "RenderObject.h"
namespace NCL {
	using namespace NCL::Rendering;
	namespace CSC8503 {
		class UniformsRenderObject :
			public RenderObject
		{
		public:
			UniformsRenderObject(Transform* parentTransform, MeshGeometry* mesh, TextureBase* tex, ShaderBase* shader);
			~UniformsRenderObject();

			virtual void SendUniforms();

		protected:


		};
	}
}

