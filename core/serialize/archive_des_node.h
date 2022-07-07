#pragma once

#include <sstream>
#include <vector>
#include <unordered_map>
#include <stdexcept>

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>

#include "serdes_utils.h"

using namespace serialize;

class Archive;

class ArchiveDesNodeBase {
public:
	ArchiveDesNodeBase(std::size_t id, std::string name) {
		id_ = id;
		name_ = name;
	}

	std::size_t Id()
	{
		return id_;
	}

	std::string Name()
	{
		return name_;
	}

	virtual void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node) {}

	virtual void ConstructFromDeserializedDependencies() {}

	virtual std::vector<ArchiveDesNodeBase*> GetChildArchiveNodes(Archive& archive, rapidxml::xml_node<>& xml_node) { return {}; };

protected:
	std::size_t id_;
	std::string name_;
};

template<typename T, typename Enable = void>
class ArchiveDesObjectNode : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, std::string name, T& object)
		: ArchiveDesNodeBase(id, name)
		, object_(object) 
	{
		std::string type_name = typeid(T).name();
		throw std::runtime_error(std::string("Deserialization not implemented for object: '") + name + std::string("' with type: ") + type_name);
	}

private:
	T& object_;
};

template<typename T>
class ArchiveDesObjectNode<T, typename std::enable_if<std::is_arithmetic<T>::value>::type> : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, std::string name, T& object)
		: ArchiveDesNodeBase(id, name)
		, object_(object) {}

	void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node) override
	{
		ArchiveDesNodeBase::DeserializeHumanReadable(xml_node);
		serialize::DeserializeArithmeticFromString(object_, xml_node.value());
	}

private:
	T& object_;
};

template<typename T>
class ArchiveDesObjectNode<T, typename std::enable_if<std::is_enum<T>::value>::type> : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, std::string name, T& object)
		: ArchiveDesNodeBase(id, name)
		, object_(object) {}

	void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node) override
	{
		ArchiveDesNodeBase::DeserializeHumanReadable(xml_node);
		int object_enum_value;
		serialize::DeserializeArithmeticFromString(object_enum_value, xml_node.value());
		object_ = (T)object_enum_value;
	}

private:
	T& object_;
};

template<class T, std::size_t N>
class ArchiveDesObjectNode<T[N]> : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, std::string name, T (&obj_array)[N])
		: ArchiveDesNodeBase(id, name)
		, obj_array_(obj_array) {}

	std::vector<ArchiveDesNodeBase*> GetChildArchiveNodes(Archive& archive, rapidxml::xml_node<>& xml_node) override
	{
		std::vector<ArchiveDesNodeBase*> children(N);
		rapidxml::xml_node<>* child_node = xml_node.first_node();
		for (std::size_t i = 0; i < N; ++i) {
			children[i] = archive.RegisterObjectForDeserialization(*child_node, obj_array_[i]);
			child_node = child_node->next_sibling();
		}
		return children;
	}

private:
	T (&obj_array_)[N];
};

template<>
class ArchiveDesObjectNode<std::string> : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, std::string name, std::string& obj_string)
		: ArchiveDesNodeBase(id, name)
		, obj_string_(obj_string) {}

	void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node) override
	{
		ArchiveDesNodeBase::DeserializeHumanReadable(xml_node);
		obj_string_ = std::string(xml_node.value());
	}

private:
	std::string& obj_string_;
};

template<typename T>
class ArchiveDesObjectNode<std::vector<T>> : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, std::string name, std::vector<T>& obj_vector)
		: ArchiveDesNodeBase(id, name)
		, obj_vector_(obj_vector) {}

	void DeserializeHumanReadable(rapidxml::xml_node<>& xml_node) override
	{
		std::size_t count;
		DeserializeArithmeticFromString(count, xml_node.last_attribute("count")->value());
		obj_vector_.resize(count);
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
	ArchiveDesObjectNode(std::size_t id, std::string name, std::unordered_map<T, U>& obj_unordered_map)
		: ArchiveDesNodeBase(id, name)
		, obj_unordered_map_(obj_unordered_map) {}

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
class ArchiveDesObjectNode<std::map<T, U>> : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, std::string name, std::map<T, U>& obj_map)
		: ArchiveDesNodeBase(id, name)
		, obj_map_(obj_map) {}

	void ConstructFromDeserializedDependencies() override
	{
		for (std::pair<T, U> map_pair : contents_) {
			obj_map_.insert(map_pair);
		}
	}

	std::vector<ArchiveDesNodeBase*> GetChildArchiveNodesForDeserialization(Archive& archive, rapidxml::xml_node<>& xml_node) override
	{
		return { archive.RegisterObjectForDeserialization(xml_node, contents_) };
	}

private:
	std::map<T, U>& obj_map_;
	std::vector<std::pair<T, U>> contents_;
};

template<typename T, typename U>
class ArchiveDesObjectNode<std::pair<T, U>> : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, std::string name, std::pair<T, U>& obj_pair)
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
	ArchiveDesObjectNode(std::size_t id, std::string name, std::shared_ptr<T>& obj_shared_ptr)
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

template<class...> using void_t = void;

template<class, class = void>
struct deserializes_members : std::false_type {};
template<class T>
struct deserializes_members<T, void_t<decltype(std::declval<T>().RegisterDeserializableMembers(std::declval<Archive&>(), std::declval<rapidxml::xml_node<>&>()))>> : std::true_type {};

template<class, class = void>
struct implements_construct_from_deserialized_deps : std::false_type {};
template<class T>
struct implements_construct_from_deserialized_deps<T, void_t<decltype(std::declval<T>().ConstructFromDeserializedDependencies())>> : std::true_type {};

template<typename T>
class ArchiveDesObjectNode<T, typename std::enable_if<deserializes_members<T>::value>::type> : ArchiveDesNodeBase {
public:
	ArchiveDesObjectNode(std::size_t id, std::string name, T& object)
		: ArchiveDesNodeBase(id, name)
		, object_(object) {}

	template<typename U = T>
	typename std::enable_if<implements_construct_from_deserialized_deps<U>::value, void>::type
		ConstructFromDeserializedDependencies()
	{
		object_.ConstructFromDeserializedDependencies();
	}

	std::vector<ArchiveDesNodeBase*> GetChildArchiveNodes(Archive& archive, rapidxml::xml_node<>& xml_node) override
	{
		return object_.RegisterDeserializableMembers(archive, xml_node);
	}

private:
	T& object_;
};

class ArchiveDesPointerNodeBase : public ArchiveDesNodeBase {
public:
	ArchiveDesPointerNodeBase(std::size_t id, std::string name, std::size_t pointee_id)
		: ArchiveDesNodeBase(id, name) {
		pointee_id_ = pointee_id;
	}

	virtual void DidRegisterPointee(void* ptr, std::size_t pointee_id) = 0;

	virtual void DynamicallyAllocateObject(rapidxml::xml_node<>& xml_node) = 0;

	virtual ArchiveDesNodeBase* PointeeNode(Archive& archive, rapidxml::xml_node<>& xml_node) = 0;

protected:
	std::size_t pointee_id_;
};

template<class, class = void>
struct dynamically_allocates_derived : std::false_type {};

template<class T>
struct dynamically_allocates_derived<T, void_t<decltype(std::declval<T*>().DynamicallyAllocatedDerivedObject(std::declval<rapidxml::xml_node<>&>()))>> : std::true_type {};

template<class T>
typename std::enable_if<!dynamically_allocates_derived<T>::value, void>::type 
DynamicallyAllocate(T* object_ptr, rapidxml::xml_node<>& xml_node)
{
	object_ptr = new T();
}

template<class T>
typename std::enable_if<dynamically_allocates_derived<T>::value, void>::type
DynamicallyAllocate(T* object_ptr, rapidxml::xml_node<>& xml_node)
{
	object_ptr = T::DynamicallyAllocatedDerivedObject(xml_node);
}

template<typename T>
class ArchiveDesPointerNode : public ArchiveDesPointerNodeBase {
public:
	ArchiveDesPointerNode(std::size_t id, std::string name, std::size_t pointee_id, T*& object_ptr)
		: ArchiveDesPointerNodeBase(id, name, pointee_id)
		, object_ptr_(object_ptr) {}

	void DidRegisterPointee(void* ptr, std::size_t pointee_id) override
	{
		object_ptr_ = static_cast<T*>(ptr);
		pointee_id_ = pointee_id;
	}

	void DynamicallyAllocateObject(rapidxml::xml_node<>& xml_node) override
	{
		DynamicallyAllocate(object_ptr_, xml_node);
	}

	ArchiveDesNodeBase* PointeeNode(Archive& archive, rapidxml::xml_node<>& xml_node) override
	{
		return archive.RegisterObjectForDeserialization(xml_node, *object_ptr_);
	}

private:
	T*& object_ptr_;
};