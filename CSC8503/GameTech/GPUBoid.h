#pragma once
#include "../CSC8503Common/ComputeGameObject.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
namespace NCL {
	namespace CSC8503 {
		class GPUBoid :
			public ComputeGameObject
		{
		public:
			GPUBoid() {};
			GPUBoid(float x, float z, OGLMesh* mesh, OGLShader* shader);
			~GPUBoid() {};

			virtual void OnSetup();
			virtual void OnDraw();

		protected:
			OGLComputeShader* flockShader;

			GLuint flockSSBO[2]; // will be used as the persistent buffer in this case
								// double buffering

			Vector3* flockPtrFirst;
			Vector3* flockPtrSecond;

			GLuint bufferIndex = 0;
			GLuint flags;
		};
	}
}


