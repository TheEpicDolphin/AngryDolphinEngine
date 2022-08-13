#pragma once

#include <sstream>
#include <vector>
#include <unordered_map>
#include <map>
#include <stdexcept>

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>

#include "serdes_utils.h"

enum class VariableTypeCategory
{
	Unknown = 0,
	Arithmetic,
	Enum,
	Pointer,
	Array,
	// StaticArray,
	// DynamicArray,
	Class,
	// Union,
};

class IVariable {
public:
	virtual std::string Name() = 0;
	virtual std::string TypeName() = 0;
	virtual void* ObjectHandle() = 0;
	virtual VariableTypeCategory TypeCategory() = 0;
};

class IArithmeticVariable : IVariable {
public:
	virtual std::string ReadValueAsString() = 0;
	virtual void SetValueFromString(const char* value_as_string) = 0;
	virtual std::vector<char> ReadValueAsBytes() = 0;
	virtual void SetValueFromBytes(const char* bytes) = 0;
	virtual int Size() = 0;
};

template<typename T>
class ArithmeticVariable : IArithmeticVariable {
public:
	ArithmeticVariable(std::string name, T& object)
		, name_(name)
		, object_(object) {}

	std::string Name() override {
		return name_;
	}

	void* ObjectHandle() override {
		return &object_;
	}

	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::Arithmetic;
	}

	std::string ReadValueAsString() override {
		return serialize::SerializeArithmeticToString(object_);
	}

	void SetValueFromString(const char* value_as_string) override {
		serialize::DeserializeArithmeticFromString(object_, value_as_string);
	}

	std::vector<char> ReadValueAsBytes() override {
		return serialize::SerializeArithmeticToBytes(object_);
	}

	void SetValueFromBytes(const char* bytes) override {
		serialize::DeserializeArithmeticFromBytes(object_, bytes);
	}

	int Size() override {
		return sizeof(T);
	}

private:
	std::string name_;
	T& object_;
};

class IEnumVariable : IVariable {
public:
	virtual int GetValue() = 0;
	virtual void SetValue(int value) = 0;
};

template<typename T>
class EnumVariable : IEnumVariable {
public:
	EnumVariable(std::string name, T& object_enum_value)
		, name_(name)
		, object_enum_value_(object_enum_value) {}

	std::string Name() override {
		return name_;
	}

	void* ObjectHandle() override {
		return &object_enum_value_;
	}

	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::Enum;
	}

	int GetValue() override {
		return (int)object_enum_value_;
	}

	virtual void SetValue(int value) override {
		object_enum_value_ = (T)value;
	}

private:
	std::string name_;
	T& object_enum_value_;
};

class IPointerVariable : IVariable {
public:
	virtual void* ReferencedObjectHandle() = 0;
	virtual std::shared_ptr<IVariable> ReferencedVariable() = 0;
	virtual void SetValue(void* ptr) = 0;
};

template<typename T>
class PointerVariable : IPointerVariable {
public:
	PointerVariable(std::string name, T*& object_ptr)
		, name_(name)
		, object_ptr_(object_ptr_) {}

	std::string Name() {
		return name_;
	}

	void* ObjectHandle() {
		return &object_ptr;
	}

	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::Pointer;
	}

	void* ReferencedObjectHandle() override {
		return object_ptr;
	}

	std::shared_ptr<IVariable> ReferencedVariable() override {
		return serialize::CreateSerializerNode("object", *object_ptr_);
	}

	virtual void SetValue(void* ptr) {
		object_ptr_ = static_cast<T*>(ptr);
	}

protected:
	std::string name_;
	T*& object_ptr_;
};

class IArrayVariable : IVariable {
public:
	virtual std::size_t Length() = 0;
	virtual std::vector<std::shared_ptr<IVariable>> Elements() = 0;
};

template<class T, std::size_t N>
class ArrayVariable : IArrayVariable {
public:
	ArrayVariable(std::string name, T(&obj_array)[N])
		, name_(name)
		, obj_array_(obj_array)
	{
		for (std::size_t i = 0; i < N; ++i) {
			std::stringstream element_name_ss;
			element_name_ss << "element_" << i;
			element_names_[i] = element_name_ss.str();
		}
	}

	std::string Name() override {
		return name_;
	}

	void* ObjectHandle() override {
		return obj_array_;
	}

	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::Array;
	}

	std::size_t Length() override {
		return N;
	}

	std::vector<std::shared_ptr<IVariable>> Elements() override {
		std::vector<std::shared_ptr<IVariable>> children(N);
		for (std::size_t i = 0; i < N; ++i) {
			children[i] = serialize::CreateSerializerNode(element_names_[i], obj_array_[i]);
		}
		return children;
	}

private:
	std::string name_;
	T(&obj_array_)[N];
	std::string element_names_[N];
};

class IClassVariable : IVariable {
public:
	virtual std::vector<std::shared_ptr<IVariable>> MemberVariables() = 0;
	virtual void ConstructFromDeserializedMembers() = 0;
};

template<typename T>
class ClassVariable : IClassVariable {
public:
	ClassVariable(std::string name, T& object)
		, name_(name)
		, object_(object) {}

	std::string Name() {
		return name_;
	}

	void* ObjectHandle() {
		return &object;
	}

	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::Class;
	}

	std::vector<std::shared_ptr<IVariable>> MemberVariables() override {
		return object_.SerializableMemberVariables();
	}

	virtual void OnDeserializedAllMembers() override {
		return object_.OnDeserializedAllMembers();
	}

private:
	std::string name_;
	T& object_;
};
