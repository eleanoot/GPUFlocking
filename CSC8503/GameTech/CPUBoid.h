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

			CPUBoid(float x, float z);
		
			~CPUBoid() {
				
			};

			Vector3 Separation(std::vector<CPUBoid*> boids);
			Vector3 Alignment(std::vector<CPUBoid*> boids);
			Vector3 Cohesion(std::vector<CPUBoid*> boids);
			Vector3 Seek(Vector3 v);
			void Update(std::vector<CPUBoid*> boids);

			void ApplyForce(Vector3 force);

		protected:
			Vector3 pos;
			Vector3 vel;
			Vector3 accel;

			float maxSpeed;
			float maxForce;
		};
	}
}

