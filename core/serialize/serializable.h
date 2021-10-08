#pragma once

class ArchiveNodeBase;
class Archive;

class ISerializable
{
public:
	virtual std::vector<ArchiveNodeBase *> RegisterMemberVariablesForSerialization(Archive& archive) = 0;

	virtual std::vector<ArchiveNodeBase*> RegisterMemberVariablesForDeserialization(Archive& archive, rapidxml::xml_node<>& xml_node) = 0;
};