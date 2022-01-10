#pragma once

#include <queue>

#include "uid_generator.h"

/// Returned UIDs are NOT reused. A checked-out UID is guaranteed to never
/// repeat again for the duration of the program, unless all 2^32 - 1 possible
/// values are used (unlikely).
class NonReusableUIDGenerator : UIDGenerator {

public:
	NonReusableUIDGenerator()
	{
		/* 0 is reserved for invalid id */
		id_count_ = 1;
	}

	~NonReusableUIDGenerator() {}

	UID CheckoutNewId() override {
		return id_count_++;
	}

	void ReturnId(UID id) override {}

private:
	std::queue<UID> unused_ids_;
	UID id_count_;
};