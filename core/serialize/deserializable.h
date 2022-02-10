#pragma once

#include "rapidxml.hpp"

class ArchiveDesNodeBase;
class Archive;

class IDeserializable
{
public:

	virtual void ConstructFromDeserializedDependencies() {};

	virtual std::vector<ArchiveDesNodeBase*> RegisterMemberVariablesForDeserialization(Archive& archive, rapidxml::xml_node<>& xml_node) = 0;
};