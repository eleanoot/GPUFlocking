#include "Obstacle.h"
using namespace NCL;
using namespace CSC8503;

void Obstacle::UpdateObstacle(float dt)
{
	Vector3 newPos = startPos + Vector3(0, 0, sin(count) * 50);
	renderObject->GetTransform()->SetWorldPosition(newPos);
	count += 0.1;
}