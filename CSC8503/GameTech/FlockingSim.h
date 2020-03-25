#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/ComputeGameObject.h"
#include "FlockSystem.h"
#include "CPUBoid.h"
#include "../../Plugins/OpenGLRendering/OBJMesh.h"
#include "../../Common/Assets.h"
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
			void InitObstacles();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddGooseToWorld(const Vector3& position);
			GameObject* AddCylinderToWorld(const Vector3& position);

			GameTechRenderer* renderer;
			PhysicsSystem* physics;
			GameWorld* world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			OGLMesh* cubeMesh = nullptr;
			OGLMesh* sphereMesh = nullptr;
			OGLMesh* gooseMesh = nullptr;
			OBJMesh* cylMesh = nullptr;
			OGLTexture* basicTex = nullptr;
			OGLShader* basicShader = nullptr;
			OGLShader* instanceShader = nullptr;

			//OGLComputeShader* flockShader = nullptr;

			FlockSystem* flock;
			bool useGPU = false;
			bool useInstancing = false;

			GameObject* instanceGoose;
		};
	}
}

