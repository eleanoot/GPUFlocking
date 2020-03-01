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
			~GPUBoid() {
				delete flockShader;
				glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
			};

			virtual void OnSetup();
			virtual void OnDraw();

			struct flock_member {
				Vector3 position;
				Vector3 velocity;
			};

		protected:
			OGLComputeShader* flockShader;

			GLuint flockSSBO[2]; // will be used as the persistent buffer in this case
								// double buffering

			flock_member* flockPtrFirst;
			flock_member* flockPtrSecond;

			GLuint bufferIndex = 0;
			GLuint flags;
		};
	}
}


