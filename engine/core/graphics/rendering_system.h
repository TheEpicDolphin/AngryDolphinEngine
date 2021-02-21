#pragma once

#include <core/ecs/system.h>
#include <core/ecs/ecs.h>

class RenderingSystem : public System<RenderingSystem>
{
public:
	RenderingSystem(ECS ecs) : System<RenderingSystem>(ecs) {

	}

	void Update() 
	{
		
	}
};