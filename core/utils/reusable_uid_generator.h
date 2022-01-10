#pragma once

#include <queue>

#include "uid_generator.h"

/// Returned UIDs are reused.
class ReusableUIDGenerator : UIDGenerator {

public:
	ReusableUIDGenerator()
	{
		/* 0 is reserved for invalid id */
		id_count_ = 1;
	}

	~ReusableUIDGenerator() {}

	UID CheckoutNewId() override {
		if (unused_ids_.size() > 0) {
			UID id = unused_ids_.front();
			unused_ids_.pop();
			return id;
		}
		else {
			return id_count_++;
		}
	}

	void ReturnId(UID id) override {
		id_count_--;
		unused_ids_.push(id);
	}

private:
	std::queue<UID> unused_ids_;
	UID id_count_;
};
