#pragma once

#include <iostream>
#include <queue>

typedef std::uint64_t TypeId;

template<class T>
class TypeIdGenerator {

public:
	TypeIdGenerator() {}

	~TypeIdGenerator() {}

	TypeId CheckoutNewId() {
		if (unused_ids_.size() > 0) {
			TypeId id = unused_ids_.front();
			unused_ids_.pop();
			return id;
		}
		else {
			return id_count++;
		}
	}

	void ReturnId(TypeId id) {
		id_count_--;
		unused_ids_.push(id);
	}

private:
	std::queue<TypeId> unused_ids_;

	/* id of 0 indicates invalid id */
	static TypeId id_count_ = 1;
};