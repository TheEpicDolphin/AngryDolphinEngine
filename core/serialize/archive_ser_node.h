#pragma once

#include <sstream>
#include <vector>
#include <unordered_map>

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

class Archive;

class ArchiveSerNodeBase {
public:
	ArchiveSerNodeBase(std::size_t id, const char* name) {
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

	virtual std::vector<ArchiveSerNodeBase*> GetChildArchiveNodes(Archive& archive) = 0;

protected:
	std::size_t id_;
	const char* name_;
	rapidxml::xml_node<>* serialized_node_;
};

template<typename T, typename Enable = void>
class ArchiveSerObjectNode : ArchiveSerNodeBase {
public:
	ArchiveSerObjectNode(std::size_t id, const char* name, T& object)
		: ArchiveSerNodeBase(id, name)
		, object_(object) 
	{
		throw std::runtime_error("Archive serialization object node is not implemented for type: ", typeid(T).name());
	}

	rapidxml::xml_node<>* SerializeHumanReadable(rapidxml::xml_document<>& xml_doc) override
	{
		return ArchiveSerNodeBase::SerializeHumanReadable(xml_doc);
	}

	std::vector<ArchiveSerNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return {};
	}

private:
	T& object_;
};

template<typename T>
class ArchiveSerObjectNode<T, typename std::enable_if<std::is_arithmetic<T>::value>::type> : ArchiveSerNodeBase {
public:
	ArchiveSerObjectNode(std::size_t id, const char* name, T& object)
		: ArchiveSerNodeBase(id, name)
		, object_(object) {}

	rapidxml::xml_node<>* SerializeHumanReadable(rapidxml::xml_document<>& xml_doc) override
	{
		rapidxml::xml_node<>* base_node = ArchiveSerNodeBase::SerializeHumanReadable(xml_doc);
		std::stringstream tmp_ss;
		tmp_ss << object_;
		base_node->value(xml_doc.allocate_string(tmp_ss.str().c_str()));
		return base_node;
	}

	std::vector<ArchiveSerNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return {};
	}

private:
	T& object_;
};

template<class T, std::size_t N>
class ArchiveSerObjectNode<T[N]> : ArchiveSerNodeBase {
public:
	ArchiveSerObjectNode(std::size_t id, const char* name, T (&obj_array)[N])
		: ArchiveSerNodeBase(id, name)
		, obj_array_(obj_array) 
	{
		for (std::size_t i = 0; i < N; ++i) {
			std::stringstream element_name_ss;
			element_name_ss << "element_" << i;
			element_names_[i] = element_name_ss.str();
		}
	}

	rapidxml::xml_node<>* SerializeHumanReadable(rapidxml::xml_document<>& xml_doc) override
	{
		rapidxml::xml_node<>* base_node = ArchiveSerNodeBase::SerializeHumanReadable(xml_doc);
		rapidxml::xml_attribute<>* container_attribute = xml_doc.allocate_attribute("container_type", "array");
		base_node->append_attribute(container_attribute);
		char* array_count_string = xml_doc.allocate_string(std::to_string(N).c_str());
		rapidxml::xml_attribute<>* array_count_atttribute = xml_doc.allocate_attribute("count", array_count_string);
		base_node->append_attribute(array_count_atttribute);
		return base_node;
	}

	std::vector<ArchiveSerNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		std::vector<ArchiveSerNodeBase*> children(N);
		for (std::size_t i = 0; i < N; ++i) {
			children[i] = archive.RegisterObjectForSerialization(element_names_[i].c_str(), obj_array_[i]);
		}
		return children;
	}

private:
	T (&obj_array_)[N];
	std::string element_names_[N];
};

template<>
class ArchiveSerObjectNode<std::string> : ArchiveSerNodeBase {
public:
	ArchiveSerObjectNode(std::size_t id, const char* name, std::string& obj_string)
		: ArchiveSerNodeBase(id, name)
		, obj_string_(obj_string) {}

	rapidxml::xml_node<>* SerializeHumanReadable(rapidxml::xml_document<>& xml_doc) override
	{
		rapidxml::xml_node<>* base_node = ArchiveSerNodeBase::SerializeHumanReadable(xml_doc);
		base_node->value(obj_string_.c_str());
		return base_node;
	}

	std::vector<ArchiveSerNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return {};
	}

private:
	std::string& obj_string_;
};

template<typename T>
class ArchiveSerObjectNode<std::vector<T>> : ArchiveSerNodeBase {
public:
	ArchiveSerObjectNode(std::size_t id, const char* name, std::vector<T>& obj_vector)
		: ArchiveSerNodeBase(id, name)
		, obj_vector_(obj_vector) 
	{
		const std::size_t N = obj_vector.size();
		element_names_ = std::vector<std::string>(N);
		for (std::size_t i = 0; i < N; ++i) {
			std::stringstream element_name_ss;
			element_name_ss << "element_" << i;
			element_names_[i] = element_name_ss.str();
		}
	}

	rapidxml::xml_node<>* SerializeHumanReadable(rapidxml::xml_document<>& xml_doc) override
	{
		rapidxml::xml_node<>* base_node = ArchiveSerNodeBase::SerializeHumanReadable(xml_doc);
		rapidxml::xml_attribute<>* container_attribute = xml_doc.allocate_attribute("container_type", "vector");
		base_node->append_attribute(container_attribute);
		char* vector_count_string = xml_doc.allocate_string(std::to_string(obj_vector_.size()).c_str());
		rapidxml::xml_attribute<>* vector_count_atttribute = xml_doc.allocate_attribute("count", vector_count_string);
		base_node->append_attribute(vector_count_atttribute);
		return base_node;
	}

	std::vector<ArchiveSerNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		std::vector<ArchiveSerNodeBase*> children;
		for (std::size_t i = 0; i < obj_vector_.size(); ++i) {
			children.push_back(archive.RegisterObjectForSerialization(element_names_[i].c_str(), obj_vector_[i]));
		}
		return children;
	}

private:
	std::vector<T>& obj_vector_;
	std::vector<std::string> element_names_;
};

template<typename T, typename U>
class ArchiveSerObjectNode<std::unordered_map<T, U>> : ArchiveSerNodeBase {
public:
	ArchiveSerObjectNode(std::size_t id, const char* name, std::unordered_map<T, U>& obj_unordered_map)
		: ArchiveSerNodeBase(id, name)
		, obj_unordered_map_(obj_unordered_map) {
		for (std::pair<T, U> unordered_map_pair : obj_unordered_map_) {
			contents_.push_back(unordered_map_pair);
		}
	}

	std::vector<ArchiveSerNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return  { archive.RegisterObjectForSerialization("unordered_map_contents", contents_) };
	}

private:
	std::unordered_map<T, U>& obj_unordered_map_;
	std::vector<std::pair<T, U>> contents_;
};

template<typename T, typename U>
class ArchiveSerObjectNode<std::map<T, U>> : ArchiveSerNodeBase {
public:
	ArchiveSerObjectNode(std::size_t id, const char* name, std::map<T, U>& obj_map)
		: ArchiveSerNodeBase(id, name)
		, obj_map_(obj_map) {
		for (std::pair<T, U> map_pair : obj_map) {
			contents_.push_back(map_pair);
		}
	}

	std::vector<ArchiveSerNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return  { archive.RegisterObjectForSerialization("map_contents", contents_) };
	}

private:
	std::map<T, U>& obj_map_;
	std::vector<std::pair<T, U>> contents_;
};

template<typename T, typename U>
class ArchiveSerObjectNode<std::pair<T, U>> : ArchiveSerNodeBase {
public:
	ArchiveSerObjectNode(std::size_t id, const char* name, std::pair<T, U>& obj_pair)
		: ArchiveSerNodeBase(id, name)
		, obj_pair_(obj_pair) {}

	std::vector<ArchiveSerNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return archive.RegisterObjectsForSerialization({ "first" , obj_pair_.first }, { "second", obj_pair_.second });
	}

private:
	std::pair<T, U>& obj_pair_;
};

template<typename T>
class ArchiveSerObjectNode<std::shared_ptr<T>> : ArchiveSerNodeBase {
public:
	ArchiveSerObjectNode(std::size_t id, const char* name, std::shared_ptr<T>& obj_shared_ptr)
		: ArchiveSerNodeBase(id, name)
		, obj_shared_ptr_(obj_shared_ptr) {
		obj_ptr_ = obj_shared_ptr_.get();
	}

	std::vector<ArchiveSerNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return { archive.RegisterObjectForSerialization("ptr", obj_ptr_) };
	}

private:
	std::shared_ptr<T>& obj_shared_ptr_;
	T* obj_ptr_;
};

template<typename T>
class ArchiveSerObjectNode<T, typename std::enable_if<std::is_base_of<ISerializable, T>::value>::type> : ArchiveSerNodeBase {
public:
	ArchiveSerObjectNode(std::size_t id, const char* name, T& object)
		: ArchiveSerNodeBase(id, name)
		, object_(object) {}

	std::vector<ArchiveSerNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return object_.RegisterMemberVariablesForSerialization(archive);
	}

private:
	T& object_;
};

class ArchiveSerPointerNodeBase : public ArchiveSerNodeBase {
public:
	ArchiveSerPointerNodeBase(std::size_t id, const char* name, std::size_t pointee_id)
		: ArchiveSerNodeBase(id, name) {
		pointee_id_ = pointee_id;
	}

	rapidxml::xml_node<>* SerializeHumanReadable(rapidxml::xml_document<>& xml_doc) override
	{
		rapidxml::xml_node<>* base_node = ArchiveSerNodeBase::SerializeHumanReadable(xml_doc);
		rapidxml::xml_node<>* pointee_node = xml_doc.allocate_node(rapidxml::node_element, "pointee");
		char* pointee_id_string = xml_doc.allocate_string(std::to_string(pointee_id_).c_str());
		rapidxml::xml_attribute<>* pointee_id_atttribute = xml_doc.allocate_attribute("pointee_id", pointee_id_string);
		pointee_node->append_attribute(pointee_id_atttribute);
		base_node->append_node(pointee_node);
		return base_node;
	}

	std::vector<ArchiveSerNodeBase*> GetChildArchiveNodes(Archive& archive) override
	{
		return {};
	}

	virtual void DidRegisterPointee(rapidxml::xml_document<>& xml_doc, void* ptr, std::size_t pointee_id) = 0;

	virtual ArchiveSerNodeBase* PointeeNode(Archive& archive) = 0;

protected:
	std::size_t pointee_id_;
};

template<typename T>
class ArchiveSerPointerNode : public ArchiveSerPointerNodeBase {
public:
	ArchiveSerPointerNode(std::size_t id, const char* name, std::size_t pointee_id, T*& object_ptr)
		: ArchiveSerPointerNodeBase(id, name, pointee_id)
		, object_ptr_(object_ptr) {}

	void DidRegisterPointee(rapidxml::xml_document<>& xml_doc, void* ptr, std::size_t pointee_id) override
	{
		object_ptr_ = static_cast<T*>(ptr);
		pointee_id_ = pointee_id;
		if (serialized_node_) {
			char* pointee_id_string = xml_doc.allocate_string(std::to_string(pointee_id_).c_str());
			serialized_node_->first_node("pointee")->first_attribute("pointee_id")->value(pointee_id_string);
		}
	}

	ArchiveSerNodeBase* PointeeNode(Archive& archive) override
	{
		return archive.RegisterObjectForSerialization("object", *object_ptr_);
	}

private:
	T*& object_ptr_;
};