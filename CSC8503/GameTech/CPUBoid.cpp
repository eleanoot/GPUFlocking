#include "CPUBoid.h"

using namespace NCL;
using namespace CSC8503;

float clamp(float n, float lower, float upper) {
	return std::max(lower, std::min(n, upper));
}

Vector3 CPUBoid::GetSeperationVector(Transform target)
{
	Vector3 diff = transform.GetWorldPosition() - target.GetWorldPosition();
	float distance = diff.Length();
	float scale = clamp(1.0f - distance / 8.0f, 0, 1);
	return diff * (scale / distance);
}

void CPUBoid::UpdateBoid(float dt)
{
	Vector3 currentPos = transform.GetWorldPosition();
	// current rotation 

	Vector3 seperation = Vector3(0, 0, 0);
	Vector3 alignment = Vector3(0, 0, 1);
	Vector3 cohesion = Vector3(0,0,0);

	for each(auto boid in neighbours)
	{
		if (boid == this) continue;

		Transform t = boid->transform;
		seperation += GetSeperationVector(t);
		alignment += t.GetWorldOrientation() * Vector3(0, 0, 1);
		cohesion += t.GetWorldPosition();
	}

	float avg = 1.0f / neighbours.size();
	alignment *= avg;
	cohesion *= avg;
	cohesion = (cohesion - currentPos).Normalised();

	transform.SetWorldPosition(currentPos + (transform.GetWorldOrientation() * Vector3(0, 0, 1)) * (physicsObject->GetLinearVelocity() * dt));
}