#pragma once
#include "../CSC8503Common/GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class CPUBoid :
			public GameObject
		{
		public:
			CPUBoid(string name = "", bool physics = true) : GameObject(name, physics) {};
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

		protected:
			std::vector<CPUBoid*> neighbours;

		};
	}
}

