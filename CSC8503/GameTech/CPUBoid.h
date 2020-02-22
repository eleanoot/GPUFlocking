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
			CPUBoid(FlockSystem* sys, string name = "", bool physics = true) : GameObject(name, physics) {
				controller = sys;
			};
			~CPUBoid() {
				neighbours.clear();
			};

			void AddNeighbour(CPUBoid* n) { neighbours.push_back(n); }

			void GetNeighourIterators(
				std::vector<CPUBoid*>::const_iterator& first,
				std::vector<CPUBoid*>::const_iterator& last) {

				first = neighbours.begin();
				last = neighbours.end();
			}

			Vector3 GetSeperationVector(Transform target);

			void UpdateBoid(float dt);

		protected:
			std::vector<CPUBoid*> neighbours;
			FlockSystem* controller;
		};
	}
}

