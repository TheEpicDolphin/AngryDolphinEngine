#pragma once

#include <sstream>
#include <vector>
#include <unordered_map>

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

class Archive;

class ArchiveDesNodeBase {
public:
	ArchiveDesNodeBase(std::size_t id, const char* name) {
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

	virtual void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node) {}

	virtual void ConstructFromDeserializedDependencies() {}

	virtual std::vector<ArchiveDesNodeBase*> GetChildArchiveNodes(Archive& archive, rapidxml::xml_node<>& xml_node) = 0;

protected:
	std::size_t id_;
	const char* name_;
};

template<typename T, typename Enable = void>
class ArchiveDesObjectNode : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, const char* name, T& object)
		: ArchiveDesNodeBase(id, name)
		, object_(object) {}

	void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node) override
	{
		ArchiveDesNodeBase::DeserializeHumanReadable(xml_node);
		std::stringstream tmp_ss;
		tmp_ss << xml_node.value();
		tmp_ss >> object_;
	}

	std::vector<ArchiveDesNodeBase*> GetChildArchiveNodes(Archive& archive, rapidxml::xml_node<>& xml_node) override
	{
		return {};
	}

private:
	T& object_;
};

template<typename T>
class ArchiveDesObjectNode<T, typename std::enable_if<std::is_base_of<IDeserializable, T>::value>::type> : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, const char* name, T& object)
		: ArchiveDesNodeBase(id, name)
		, object_(object) {}

	void ConstructFromDeserializedDependencies() override
	{
		object_.ConstructFromDeserializedDependencies();
	}

	std::vector<ArchiveDesNodeBase*> GetChildArchiveNodes(Archive& archive, rapidxml::xml_node<>& xml_node) override
	{
		return object_.RegisterMemberVariablesForDeserialization(archive, xml_node);
	}

private:
	T& object_;
};

template<typename T>
class ArchiveDesObjectNode<std::vector<T>> : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, const char* name, std::vector<T>& obj_vector)
		: ArchiveDesNodeBase(id, name)
		, obj_vector_(obj_vector) {}

	void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node) override
	{
		std::size_t count;

		std::stringstream tmp_ss;
		tmp_ss << xml_node.last_attribute("count")->value();
		tmp_ss >> count;

		obj_vector_.resize(count);
	}

	void ConstructFromDeserializedDependencies() override
	{
		// no-op
	}

	std::vector<ArchiveDesNodeBase*> GetChildArchiveNodes(Archive& archive, rapidxml::xml_node<>& xml_node) override
	{
		std::vector<ArchiveDesNodeBase*> children;
		rapidxml::xml_node<>* child_node = xml_node.first_node();
		for (T& object : obj_vector_) {
			children.push_back(archive.RegisterObjectForDeserialization(*child_node, object));
			child_node = child_node->next_sibling();
		}
		return children;
	}

private:
	std::vector<T>& obj_vector_;
};

template<typename T, typename U>
class ArchiveDesObjectNode<std::unordered_map<T, U>> : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, const char* name, std::unordered_map<T, U>& obj_unordered_map)
		: ArchiveDesNodeBase(id, name)
		, obj_unordered_map_(obj_unordered_map) {}

	void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node) override
	{
		std::size_t count;

		std::stringstream tmp_ss;
		tmp_ss << xml_node.last_attribute("count")->value();
		tmp_ss >> count;

		contents_.resize(count);
	}

	void ConstructFromDeserializedDependencies() override
	{
		for (std::pair<T, U> unordered_map_pair : contents_) {
			obj_unordered_map_.insert(unordered_map_pair);
		}
	}

	std::vector<ArchiveDesNodeBase*> GetChildArchiveNodesForDeserialization(Archive& archive, rapidxml::xml_node<>& xml_node) override
	{
		return { archive.RegisterObjectForDeserialization(xml_node, contents_) };
	}

private:
	std::unordered_map<T, U>& obj_unordered_map_;
	std::vector<std::pair<T, U>> contents_;
};

template<typename T, typename U>
class ArchiveDesObjectNode<std::pair<T, U>> : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, const char* name, std::pair<T, U>& obj_pair)
		: ArchiveDesNodeBase(id, name)
		, obj_pair_(obj_pair) {}

	std::vector<ArchiveDesNodeBase*> GetChildArchiveNodesForDeserialization(Archive& archive, rapidxml::xml_node<>& xml_node) override
	{
		return archive.RegisterObjectsForDeserialization(xml_node, obj_pair_.first, obj_pair_.second);
	}

private:
	std::pair<T, U>& obj_pair_;
};

template<typename T>
class ArchiveDesObjectNode<std::shared_ptr<T>> : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, const char* name, std::shared_ptr<T>& obj_shared_ptr)
		: ArchiveDesNodeBase(id, name)
		, obj_shared_ptr_(obj_shared_ptr) {}

	void ConstructFromDeserializedDependencies() override
	{
		obj_shared_ptr_.reset(obj_ptr_);
	}

	std::vector<ArchiveDesNodeBase*> GetChildArchiveNodes(Archive& archive, rapidxml::xml_node<>& xml_node) override
	{
		return { archive.RegisterObjectForDeserialization(*xml_node.first_node(), obj_ptr_) };
	}

private:
	std::shared_ptr<T>& obj_shared_ptr_;
	T* obj_ptr_;
};

class ArchiveDesPointerNodeBase : public ArchiveDesNodeBase {
public:
	ArchiveDesPointerNodeBase(std::size_t id, const char* name, std::size_t pointee_id)
		: ArchiveDesNodeBase(id, name) {
		pointee_id_ = pointee_id;
	}

	std::vector<ArchiveDesNodeBase*> GetChildArchiveNodes(Archive& archive, rapidxml::xml_node<>& xml_node) override
	{
		return {};
	}

	virtual void DidRegisterPointee(rapidxml::xml_document<>& xml_doc, void* ptr, std::size_t pointee_id) = 0;

	virtual void DynamicallyAllocateObject() = 0;

	virtual ArchiveDesNodeBase* PointeeNode(Archive& archive, rapidxml::xml_node<>& xml_node) = 0;

protected:
	std::size_t pointee_id_;
};

template<typename T>
class ArchiveDesPointerNode : public ArchiveDesPointerNodeBase {
public:
	ArchiveDesPointerNode(std::size_t id, const char* name, std::size_t pointee_id, T*& object_ptr)
		: ArchiveDesPointerNodeBase(id, name, pointee_id)
		, object_ptr_(object_ptr) {}

	void DidRegisterPointee(rapidxml::xml_document<>& xml_doc, void* ptr, std::size_t pointee_id) override
	{
		object_ptr_ = static_cast<T*>(ptr);
		pointee_id_ = pointee_id;
	}

	void DynamicallyAllocateObject() override
	{
		object_ptr_ = new T();
	}

	ArchiveDesNodeBase* PointeeNode(Archive& archive, rapidxml::xml_node<>& xml_node) override
	{
		return archive.RegisterObjectForDeserialization(xml_node, *object_ptr_);
	}

private:
	T*& object_ptr_;
};