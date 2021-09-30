#pragma once

class Archive;

class ISerializable
{
public:
	virtual void SerializeHumanReadable(Archive& archive, std::ostream& ostream) = 0;

	virtual void DeserializeHumanReadable(Archive& archive, std::ostream& ostream) = 0;
};