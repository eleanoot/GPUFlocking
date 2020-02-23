#include "FlockSystem.h"
#include "../GameTech/CPUBoid.h"
using namespace NCL;
using namespace NCL::CSC8503;

FlockSystem::FlockSystem()
{
	
}

FlockSystem::~FlockSystem()
{
	allBoids.clear();
}

void FlockSystem::UpdateFlock(float dt)
{
	for (int i = 0; i < allBoids.size(); i++)
	{
		allBoids[i]->Update(allBoids);
		allBoids[i]->GetTransform().SetWorldPosition(allBoids[i]->GetTransform().GetWorldPosition() + allBoids[i]->GetPos());
		//float theta = allBoids[i]->Angle(allBoids[i]->GetPhysicsObject()->GetLinearVelocity());
		float theta = allBoids[i]->Angle(allBoids[i]->GetVel());
		allBoids[i]->GetRenderObject()->GetTransform()->SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), theta));
	}
	
}



