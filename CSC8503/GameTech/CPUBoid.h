#define NOMINMAX
#pragma once
#include "../CSC8503Common/GameObject.h"
#include "FlockSystem.h"
#include <algorithm>
namespace NCL {
	namespace CSC8503 {
		class CPUBoid :
			public GameObject
		{
		public:
			CPUBoid() : GameObject("BOID", true) {
				
			};

			CPUBoid(float x, float z, OGLMesh* mesh, OGLShader* shader);
		
			~CPUBoid() {
				
			};

			Vector3 Separation(std::vector<CPUBoid*> boids);
			Vector3 Alignment(std::vector<CPUBoid*> boids);
			Vector3 Cohesion(std::vector<CPUBoid*> boids);
			Vector3 Seek(Vector3 v);
			Vector3 Avoidance(std::vector<GameObject*> obstacles);
			void Update(std::vector<CPUBoid*> boids, std::vector<GameObject*> obstacles);

			void ApplyForce(Vector3 force);

			void Boundaries();

			float Angle(Vector3 v);

			Vector3 GetPos() const { return pos; }

			Vector3 GetVel() const { return vel; }

			Vector3 GetAcc() const { return tempAccel; }

		protected:
			Vector3 pos;
			Vector3 vel;
			Vector3 accel;

			Vector3 tempAccel;

			float maxSpeed;
			float maxForce;

			const float MAX_SEE_AHEAD = 70;

			bool LineCircleIntersect(Vector3 ahead, Vector3 ahead2, GameObject* obstacle);
		};
	}
}

