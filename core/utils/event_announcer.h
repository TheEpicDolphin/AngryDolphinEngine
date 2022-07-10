#pragma once

#include <vector>

template<typename T>
class EventAnnouncer {

public:
	void AddListener(T* listener) {
		listeners_.push_back(listener);
	}

	void RemoveListener(T* listener) {
		listeners_.erase(std::remove(listeners_.begin(), listeners_.end(), listener), listeners_.end());
	}

	template<typename F, typename... Args>
	void Announce(F func, Args... args) {
		for (T* listener : listeners_) {
			(listener->*func)(args...);
		}
	}

	std::size_t ListenerCount() {
		return listeners_.size();
	}

private:
	std::vector<T*> listeners_;
};