#pragma once

#include "archive.h"

class ISerializable
{
public:
	virtual void SerializeHumanReadable(Archive& archive, std::ostream& ostream);

	virtual void DeserializeHumanReadable(Archive& archive, std::ostream& ostream);
};