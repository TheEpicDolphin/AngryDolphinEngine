#pragma once

class ArchiveNodeBase;
class Archive;

class ISerializable
{
public:
	virtual std::vector<ArchiveNodeBase *> SerializeHumanReadable(Archive& archive) = 0;

	virtual std::vector<ArchiveNodeBase*> DeserializeHumanReadable(Archive& archive) = 0;
};