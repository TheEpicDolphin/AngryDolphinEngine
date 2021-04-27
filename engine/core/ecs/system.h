#pragma once

#include <iostream>
#include "ecs.h"

class SystemBase {

};

template<typename T>
class System : SystemBase
{
public:
	System() = default;

	System(ECS *ecs) {
		ecs_ = ecs;
	}

protected:
	ECS *ecs_;
};