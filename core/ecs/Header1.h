#pragma once

class ServiceContainer {

	template<typename T>
	void BindTo(IService service);

	std::vector<IService> services_;
};