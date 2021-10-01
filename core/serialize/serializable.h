#pragma once

class Archive;

class ISerializable
{
public:
	virtual void SerializeHumanReadable(Archive& archive, rapidxml::xml_node<>& xml_node) = 0;

	virtual void DeserializeHumanReadable(Archive& archive, rapidxml::xml_node<>& xml_node) = 0;
};