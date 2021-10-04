#pragma once

#include <sstream>
#include <vector>

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

class Archive;

class ArchiveNodeBase {
public:
	ArchiveNodeBase(std::size_t id, const char* name) {
		id_ = id;
		name_ = name;
	}

	std::size_t Id()
	{
		return id_;
	}

	const char* Name()
	{
		return name_;
	}

	virtual rapidxml::xml_node<>* SerializeHumanReadable(rapidxml::xml_document<>& xml_doc)
	{
		rapidxml::xml_node<>* object_node = xml_doc.allocate_node(rapidxml::node_element, name_);
		char* object_id_string = xml_doc.allocate_string(std::to_string(id_).c_str());
		rapidxml::xml_attribute<>* object_id_atttribute = xml_doc.allocate_attribute("id", object_id_string);
		object_node->append_attribute(object_id_atttribute);
		return object_node;
	}

	virtual std::vector<ArchiveNodeBase*> GetChildArchiveNodes(Archive& archive) = 0;

protected:
	std::size_t id_;
	const char* name_;
};

template<typename T, typename Enable = void>
class ArchiveObjectNode : ArchiveNodeBase {
public:
	ArchiveObjectNode(std::size_t id, const char* name, T& object) : ArchiveNodeBase(id, name), object_(object) {}

	rapidxml::xml_node<>* SerializeHumanReadable(rapidxml::xml_document<>& xml_doc) override
	{
		rapidxml::xml_node<>* base_node = ArchiveNodeBase::SerializeHumanReadable(xml_doc);
		std::stringstream tmp_ss;
		tmp_ss << object_;
		base_node->value(xml_doc.allocate_string(tmp_ss.str().c_str()));
		return base_node;
	}

	std::vector<ArchiveNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return {};
	}

private:
	T& object_;
};

template<typename T>
class ArchiveObjectNode<T, typename std::enable_if<std::is_base_of<ISerializable, T>::value>::type> : ArchiveNodeBase {
public:
	ArchiveObjectNode(std::size_t id, const char* name, T& object) : ArchiveNodeBase(id, name), object_(object) {}

	std::vector<ArchiveNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return object_.SerializeHumanReadable(archive);
	}

private:
	T& object_;
};

class ArchivePointerNode : ArchiveNodeBase {
public:
	ArchivePointerNode(std::size_t id, const char* name, std::size_t pointee_id) : ArchiveNodeBase(id, name) {
		pointee_id_ = pointee_id;
	}

	rapidxml::xml_node<>* SerializeHumanReadable(rapidxml::xml_document<>& xml_doc) override
	{
		rapidxml::xml_node<>* base_node = ArchiveNodeBase::SerializeHumanReadable(xml_doc);
		rapidxml::xml_node<>* pointer_node = xml_doc.allocate_node(rapidxml::node_element, "pointee");
		char* pointee_id_string = xml_doc.allocate_string(std::to_string(pointee_id_).c_str());
		rapidxml::xml_attribute<>* pointee_id_atttribute = xml_doc.allocate_attribute("pointee_id", pointee_id_string);
		pointer_node->append_attribute(pointee_id_atttribute);
		base_node->append_node(pointer_node);
		return base_node;
	}

	std::vector<ArchiveNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return {};
	}

private:
	std::size_t pointee_id_;
};

template<typename T>
class ArchiveObjectNode<std::vector<T>> : ArchiveNodeBase {
public:
	ArchiveObjectNode(std::size_t id, const char* name, std::vector<T>& obj_vector) : ArchiveNodeBase(id, name), obj_vector_(obj_vector) {
		count_ = obj_vector.size();
	}

	std::vector<ArchiveNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		std::vector<ArchiveNodeBase*> children = { archive.RegisterMember("count", count_) };
		for (T& object : obj_vector_) {
			children.push_back(archive.RegisterMember("element", object));
		}
		return children;
	}

private:
	std::vector<T>& obj_vector_;
	std::size_t count_;
};

template<typename T>
class ArchiveObjectNode<std::shared_ptr<T>> : ArchiveNodeBase {
public:
	ArchiveObjectNode(std::size_t id, const char* name, std::shared_ptr<T>& obj_shared_ptr) : ArchiveNodeBase(id, name), obj_shared_ptr_(obj_shared_ptr) {}

	std::vector<ArchiveNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return { archive.RegisterMember("ptr", obj_shared_ptr_.get()) };
	}

private:
	std::shared_ptr<T>& obj_shared_ptr_;
};