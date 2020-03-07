#pragma once

#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLComputeShader.h"
#include "GPUBoid.h"

namespace NCL {
	namespace CSC8503 {
		class CPUBoid;
		class FlockSystem
		{
		public: 
			FlockSystem();
			~FlockSystem();

			void UpdateFlock(float dt);

			int GetSize() { return allBoids.size(); };
			CPUBoid* GetBoid(int i) {
				return allBoids[i];
			};

			void AddBoid(CPUBoid* b) { allBoids.push_back(b); }

			void AddBoid(GPUBoid* b);

			void InitGPU();

			struct flock_member {
				Vector3 position;
				Vector3 velocity;
			};

		protected:
			std::vector<CPUBoid*> allBoids; // cpu flock
			std::vector<GPUBoid*> gpuBoids; // gpu flock
			std::vector<flock_member> gpuData;

			OGLComputeShader* flockShader = nullptr;
			GLuint flockSSBO;
			GLuint flags;
			
		};
	}
}

