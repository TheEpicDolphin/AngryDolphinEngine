#pragma once

typedef std::uint32_t UID;

class UIDGenerator {
public:
	virtual UID CheckoutNewId() = 0;

	virtual void ReturnId(UID id) = 0;
};