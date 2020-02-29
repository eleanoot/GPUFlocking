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

		if (i == 0)
		{
			std::cout << "Accel: " << allBoids[i]->GetAcc() << std::endl;
			std::cout << "Vel: " << allBoids[i]->GetVel() << std::endl;
			std::cout << "Pos: " << allBoids[i]->GetPos() << std::endl;
		}	
	}
	colourCount++;
}



