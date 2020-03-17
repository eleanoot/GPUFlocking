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
			void UpdateGPUFlock(float dt);

			int GetSize() { return allBoids.size(); };
			CPUBoid* GetBoid(int i) {
				return allBoids[i];
			};

			void AddBoid(CPUBoid* b) { allBoids.push_back(b); }

			void AddBoid(GPUBoid* b);

			void InitGPU();

			void InitInstanceFlock(OGLMesh* m);
			void UpdateInstanceFlock(float dt);

			struct flock_member {
				Vector3 position;
				float angle;
				Vector3 velocity;
				float s2;
				Vector3 accel;
				float s3;
			};

		protected:
			std::vector<CPUBoid*> allBoids; // cpu flock
			std::vector<GPUBoid*> gpuBoids; // gpu flock
			std::vector<flock_member> gpuData; 

			OGLComputeShader* flockShader = nullptr;
			//GLuint flockSSBO;
			GLuint flockSSBO[2];
			GLuint flags;
			flock_member* fmPtrOne;
			flock_member* fmPtrTwo;
			GLuint bufferIndex;

			OGLMesh* boidMesh;

		};
	}
}

