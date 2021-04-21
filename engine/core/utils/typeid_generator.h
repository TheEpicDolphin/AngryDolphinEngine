#pragma once

#include <queue>

typedef std::uint64_t TypeID;

template<class T>
class TypeIDGenerator {

public:
	TypeIDGenerator() {}

	~TypeIDGenerator() {}

	TypeID CheckoutNewId() {
		if (unused_ids_.size() > 0) {
			TypeID id = unused_ids_.front();
			unused_ids_.pop();
			return id;
		}
		else {
			return id_count_++;
		}
	}

	void ReturnId(TypeID id) {
		id_count_--;
		unused_ids_.push(id);
	}

private:
	std::queue<TypeID> unused_ids_;

	static TypeID id_count_;
};