#pragma once
#include "../CSC8503Common/GameObject.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"

#define FLOCK_SEPARATION	10
#define FLOCK_ALIGNMENT		10
#define FLOCK_COHESION		10

namespace NCL {
	namespace CSC8503 {
		class FlockSystem
		{
		public: 
			FlockSystem(int noOfBoids, GameWorld* world, OGLMesh* mesh, OGLShader* shader);
			~FlockSystem();

			void UpdateFlock() {};

			void Separation(GameObject* b) {};
			void Alignment(GameObject* b) {};
			void Cohesion(GameObject* b) {};

		protected:
			std::vector<GameObject*> allBoids; // cpu flock: will contain position and velocity to edit through physics objects 

			Vector4 colours[3] = { Vector4(1,0,0,1), Vector4(0,1,0,1), Vector4(0,0,1,1) };

			OGLMesh* boidMesh = nullptr;
		};
	}
}

