#pragma once

#include <iostream>
#include "ecs.h"

class SystemBase {

};

template<class... Ts>
class System : SystemBase
{
public:
	System(ECS ecs) {
		ecs_ = ecs;
	}

	virtual void Update() {}

private:
	ECS ecs_;
};