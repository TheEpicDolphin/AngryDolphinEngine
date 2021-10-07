#pragma once

#include <sstream>
#include <vector>
#include <unordered_map>

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

class Archive;

class ArchiveNodeDepending {
public:

};

class ArchiveNodeBase {
public:
	ArchiveNodeBase(std::uint32_t id, const char* name) {
		id_ = id;
		name_ = name;
	}

	std::uint32_t Id()
	{
		return id_;
	}

	const char* Name()
	{
		return name_;
	}

	virtual rapidxml::xml_node<>* SerializeHumanReadable(rapidxml::xml_document<>& xml_doc)
	{
		if (serialized_node_) {
			// Throw warning. This shouldn't be serialized twice.
		}

		rapidxml::xml_node<>* object_node = xml_doc.allocate_node(rapidxml::node_element, name_);
		char* object_id_string = xml_doc.allocate_string(std::to_string(id_).c_str());
		rapidxml::xml_attribute<>* object_id_atttribute = xml_doc.allocate_attribute("id", object_id_string);
		object_node->append_attribute(object_id_atttribute);

		serialized_node_ = object_node;
		return serialized_node_;
	}

	virtual void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node)
	{
		if (!strcmp(name_, xml_node.name())) {
			// Throw warning saying that names do not match
		}

		//if (id_ != xml_node.first_attribute("id")->value()) {
			// Throw error saying that ordering is not the same.
		//}
	}

	virtual void ConstructFromDeserializedDependencies() {}

	virtual std::vector<ArchiveNodeBase*> GetChildArchiveNodes(Archive& archive) = 0;

protected:
	std::uint32_t id_;
	const char* name_;
	rapidxml::xml_node<>* serialized_node_;
};

template<typename T, typename Enable = void>
class ArchiveObjectNode : ArchiveNodeBase {
public:
	ArchiveObjectNode(std::uint32_t id, const char* name, T& object) 
		: ArchiveNodeBase(id, name)
		, object_(object) {}

	rapidxml::xml_node<>* SerializeHumanReadable(rapidxml::xml_document<>& xml_doc) override
	{
		rapidxml::xml_node<>* base_node = ArchiveNodeBase::SerializeHumanReadable(xml_doc);
		std::stringstream tmp_ss;
		tmp_ss << object_;
		base_node->value(xml_doc.allocate_string(tmp_ss.str().c_str()));
		return base_node;
	}

	void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node) override 
	{
		ArchiveNodeBase::DeserializeHumanReadable(xml_node);
		std::stringstream tmp_ss;
		tmp_ss << xml_node.value();
		tmp_ss >> object_;
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
	ArchiveObjectNode(std::uint32_t id, const char* name, T& object) 
		: ArchiveNodeBase(id, name)
		, object_(object) {}

	std::vector<ArchiveNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return object_.SerializeHumanReadable(archive);
	}

private:
	T& object_;
};

template<typename T>
class ArchiveObjectNode<std::vector<T>> : ArchiveNodeBase {
public:
	ArchiveObjectNode(std::uint32_t id, const char* name, std::vector<T>& obj_vector) 
		: ArchiveNodeBase(id, name)
		, obj_vector_(obj_vector) {}

	rapidxml::xml_node<>* SerializeHumanReadable(rapidxml::xml_document<>& xml_doc) override
	{
		rapidxml::xml_node<>* base_node = ArchiveNodeBase::SerializeHumanReadable(xml_doc);
		char* vector_count_string = xml_doc.allocate_string(std::to_string(obj_vector_.size()).c_str());
		rapidxml::xml_attribute<>* vector_count_atttribute = xml_doc.allocate_attribute("count", vector_count_string);
		base_node->append_attribute(vector_count_atttribute);
		return base_node;
	}

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

	std::vector<ArchiveNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		std::vector<ArchiveNodeBase*> children;
		for (T& object : obj_vector_) {
			children.push_back(archive.RegisterMember("element", object));
		}
		return children;
	}

private:
	std::vector<T>& obj_vector_;
};

template<typename T, typename U>
class ArchiveObjectNode<std::unordered_map<T, U>> : ArchiveNodeBase {
public:
	ArchiveObjectNode(std::uint32_t id, const char* name, std::unordered_map<T, U>& obj_unordered_map)
		: ArchiveNodeBase(id, name)
		, obj_unordered_map_(obj_unordered_map) {
		for (std::pair<T, U> unordered_map_pair : obj_unordered_map_) {
			contents_.push_back(unordered_map_pair);
		}
	}

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

	std::vector<ArchiveNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return  { archive.RegisterMember("unordered_map_contents", contents_) };
	}

private:
	std::unordered_map<T, U>& obj_unordered_map_;
	std::vector<std::pair<T, U>> contents_;
};

template<typename T, typename U>
class ArchiveObjectNode<std::pair<T, U>> : ArchiveNodeBase {
public:
	ArchiveObjectNode(std::uint32_t id, const char* name, std::pair<T, U>& obj_pair)
		: ArchiveNodeBase(id, name)
		, obj_pair_(obj_pair) {}

	std::vector<ArchiveNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return archive.RegisterMembers({ "first" , obj_pair_.first }, { "second", obj_pair_.second });
	}

private:
	std::pair<T, U>& obj_pair_;
};

template<typename T>
class ArchiveObjectNode<std::shared_ptr<T>> : ArchiveNodeBase {
public:
	ArchiveObjectNode(std::uint32_t id, const char* name, std::shared_ptr<T>& obj_shared_ptr) 
		: ArchiveNodeBase(id, name)
		, obj_shared_ptr_(obj_shared_ptr) {
		obj_ptr_ = obj_shared_ptr_.get();
	}

	void ConstructFromDeserializedDependencies() override
	{
		obj_shared_ptr_ = std::make_shared<T>(obj_ptr_);
	}

	std::vector<ArchiveNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return { archive.RegisterMember("ptr", obj_ptr_) };
	}

private:
	std::shared_ptr<T>& obj_shared_ptr_;
	T* obj_ptr_;
};

class ArchivePointerNodeBase : public ArchiveNodeBase {
public:
	ArchivePointerNodeBase(std::uint32_t id, const char* name, std::uint32_t pointee_id)
		: ArchiveNodeBase(id, name) {
		pointee_id_ = pointee_id;
	}

	rapidxml::xml_node<>* SerializeHumanReadable(rapidxml::xml_document<>& xml_doc) override
	{
		rapidxml::xml_node<>* base_node = ArchiveNodeBase::SerializeHumanReadable(xml_doc);
		rapidxml::xml_node<>* pointee_node = xml_doc.allocate_node(rapidxml::node_element, "pointee");
		char* pointee_id_string = xml_doc.allocate_string(std::to_string(pointee_id_).c_str());
		rapidxml::xml_attribute<>* pointee_id_atttribute = xml_doc.allocate_attribute("pointee_id", pointee_id_string);
		pointee_node->append_attribute(pointee_id_atttribute);
		base_node->append_node(pointee_node);
		return base_node;
	}

	std::vector<ArchiveNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return {};
	}

	virtual void DidRegisterPointee(rapidxml::xml_document<>& xml_doc, void* ptr, std::uint32_t pointee_id) = 0;

	virtual void DynamicallyAllocateObject() = 0;

	virtual ArchiveNodeBase* PointeeNode(Archive& archive) = 0;

protected:
	std::size_t pointee_id_;
};

template<typename T>
class ArchivePointerNode : public ArchivePointerNodeBase {
public:
	ArchivePointerNode(std::uint32_t id, const char* name, std::uint32_t pointee_id, T*& object_ptr)
		: ArchivePointerNodeBase(id, name, pointee_id)
		, object_ptr_(object_ptr) {}

	void DidRegisterPointee(rapidxml::xml_document<>& xml_doc, void* ptr, std::uint32_t pointee_id) override 
	{
		object_ptr_ = static_cast<T*>(ptr);
		pointee_id_ = pointee_id;
		if (serialized_node_) {
			char* pointee_id_string = xml_doc.allocate_string(std::to_string(pointee_id_).c_str());
			serialized_node_->first_node("pointee")->first_attribute("pointee_id")->value(pointee_id_string);
		}
	}

	void DynamicallyAllocateObject() override
	{
		object_ptr_ = new T();
	}

	ArchiveNodeBase* PointeeNode(Archive& archive) override
	{
		return archive.RegisterMember("object", *object_ptr_);
	}

private:
	T*& object_ptr_;
};