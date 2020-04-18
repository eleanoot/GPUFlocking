#pragma once

#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLComputeShader.h"
#include "GPUBoid.h"
#include "Obstacle.h"
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

			void AddObstacle(Obstacle* o) { 
				obstacles.push_back(o); 
				obstacleData.push_back(obstacle{ o->GetTransform().GetWorldPosition(), o->GetTransform().GetLocalScale().x });
			}

			void InitGPU();

			void InitInstanceFlock(OGLMesh* m, RenderObject* r);
			void UpdateInstanceFlock(float dt);

			void InitPartitionFlock();

			void SetFlockSize(int size) { flockSize = size; }

			struct flock_member {
				Vector3 position;
				float angle;
				Vector3 velocity;
				float groupNo;
				Vector3 accel;
				float s3;
			};

			struct obstacle {
				Vector3 centre;
				float radius;
			};

			struct range {
				GLuint offset;
				GLuint count;
			};

		protected:
			std::vector<CPUBoid*> allBoids; // cpu flock
			std::vector<GPUBoid*> gpuBoids; // gpu flock
			std::vector<flock_member> gpuData; 
			std::vector<obstacle> obstacleData;

			OGLComputeShader* flockShader = nullptr;
			GLuint flockSSBO[2];
			GLuint obstacleSSBO;
			GLuint flags;
			flock_member* fmPtrOne;
			flock_member* fmPtrTwo;
			GLuint bufferIndex;

			OGLMesh* boidMesh;

			std::vector<Obstacle*> obstacles;
			obstacle* obPtr;

			int flockSize;

			float sepDis;
			float alignDis;
			float cohDis;

			// For partitioning 
			float cellSize;
			float cellRatio;
			Vector2 cellCounts;
			GLuint cellCount;

			OGLComputeShader* cellCountShader = nullptr;
			OGLComputeShader* indexShader = nullptr;
			GLuint countsBuffer;
			GLuint offsetsBuffer;
			GLuint rangesBuffer;
			GLuint indexBuffer;

		};
	}
}

