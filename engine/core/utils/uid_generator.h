#pragma once

#include <queue>

typedef std::uint64_t UID;

class UIDGenerator {

public:
	UIDGenerator() 
	{
		/* 0 is reserved for invalid id */
		id_count_ = 1;
	}

	~UIDGenerator() {}

	UID CheckoutNewId() {
		if (unused_ids_.size() > 0) {
			UID id = unused_ids_.front();
			unused_ids_.pop();
			return id;
		}
		else {
			return id_count_++;
		}
	}

	void ReturnId(UID id) {
		id_count_--;
		unused_ids_.push(id);
	}

private:
	std::queue<UID> unused_ids_;
	UID id_count_;
};