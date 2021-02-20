#pragma once

#include <iostream>
#include <queue>

typedef std::uint64_t UID;

template<class T>
class UIDGenerator {

public:
	UIDGenerator() {}

	~UIDGenerator() {}

	UID CheckoutNewId() {
		if (unused_ids_.size() > 0) {
			TypeId id = unused_ids_.front();
			unused_ids_.pop();
			return id;
		}
		else {
			return id_count++;
		}
	}

	void ReturnId(UID id) {
		id_count_--;
		unused_ids_.push(id);
	}

private:
	std::queue<UID> unused_ids_;

	/* id of 0 indicates invalid id */
	static UID id_count_ = 1;
};