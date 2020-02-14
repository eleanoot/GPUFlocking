#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/ComputeGameObject.h"
namespace NCL {
	namespace CSC8503 {
		class FlockingSim
		{
		public:
			FlockingSim();
			~FlockingSim();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();
			void InitCamera();
			void UpdateKeys();
			void InitWorld();

			GameObject* AddFloorToWorld(const Vector3& position);

			GameTechRenderer* renderer;
			PhysicsSystem* physics;
			GameWorld* world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			OGLMesh* cubeMesh = nullptr;
			OGLMesh* sphereMesh = nullptr;
			OGLTexture* basicTex = nullptr;
			OGLShader* basicShader = nullptr;
		};
	}
}

