#pragma once

class ArchiveNodeBase;
class Archive;

class ISerializable
{
public:
	virtual std::vector<ArchiveNodeBase *> RegisterMemberVariables(Archive& archive) = 0;
};