#pragma once
#include "../CSC8503Common/GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class CPUBoid :
			public GameObject
		{
		public:
			CPUBoid(string name = "", bool physics = true) : GameObject(name, physics) {};
			~CPUBoid();

		protected:
			std::vector<CPUBoid*> neighbours;

		};
	}
}

