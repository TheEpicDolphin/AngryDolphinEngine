#pragma once

class ArchiveSerNodeBase;
class Archive;

class ISerializable
{
public:
	virtual std::vector<ArchiveSerNodeBase*> RegisterMemberVariablesForSerialization(Archive& archive) = 0;
};