#pragma once
#include "..\CSC8503Common\GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class Obstacle :
			public GameObject
		{
		public:
			Obstacle() : GameObject() {};
			~Obstacle() {};
			void UpdateObstacle(float dt);

			void SetStartPos(Vector3 pos) { startPos = pos; }

		private:
			Vector3 startPos;
			float count = 0;
		};
	}
}

