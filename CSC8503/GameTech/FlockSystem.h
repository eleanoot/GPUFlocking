#pragma once
#include "../GameTech/CPUBoid.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"

#define FLOCK_SEPARATION	25.0f
#define FLOCK_ALIGNMENT		0.125f
#define FLOCK_COHESION		0.01f
#define NEIGHBOUR_RADIUS	8.0f

namespace NCL {
	namespace CSC8503 {
		class FlockSystem
		{
		public: 
			FlockSystem(int noOfBoids, GameWorld* world, OGLMesh* mesh, OGLShader* shader);
			~FlockSystem();

			void UpdateFlock(float dt);
			void FindNeighbours();

			Vector3 Separation(CPUBoid* b);
			Vector3 Alignment(CPUBoid* b);
			Vector3 Cohesion(CPUBoid* b);

		protected:
			std::vector<CPUBoid*> allBoids; // cpu flock: will contain position and velocity to edit through physics objects 

			Vector4 colours[3] = { Vector4(1,0,0,1), Vector4(0,1,0,1), Vector4(0,0,1,1) };

			OGLMesh* boidMesh = nullptr;
		};
	}
}

