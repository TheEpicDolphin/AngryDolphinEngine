#pragma once

#include <sstream>
#include <vector>
#include <unordered_map>
#include <map>
#include <stdexcept>

#include "serdes_utils.h"

enum class VariableTypeCategory
{
	Unknown = 0,
	Arithmetic,
	Enum,
	Pointer,
	Array,
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
	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::Arithmetic;
	}

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
		: name_(name)
		, object_(object) {
		static_assert(std::is_enum<T>, "T is not an arithmetic type.");
	}

	std::string Name() override {
		return name_;
	}

	void* ObjectHandle() override {
		return &object_;
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
	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::Enum;
	}

	virtual int GetValue() = 0;
	virtual void SetValue(int value) = 0;
};

template<typename T>
class EnumVariable : IEnumVariable {
public:
	EnumVariable(std::string name, T& object_enum_value)
		: name_(name)
		, object_enum_value_(object_enum_value) {
		static_assert(std::is_enum<T>, "T is not an enum type.");
	}

	std::string Name() override {
		return name_;
	}

	void* ObjectHandle() override {
		return &object_enum_value_;
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
	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::Pointer;
	}

	virtual void* PointedObjectHandle() = 0;
	virtual bool IsOwning() = 0;
	virtual void SetValue(void* ptr) = 0;
	virtual std::shared_ptr<IVariable> PointedVariable() = 0;
};

template<typename T>
class PointerVariable : IPointerVariable {
public:
	PointerVariable(std::string name, T*& object_ptr, bool is_owning)
		: name_(name)
		, object_ptr_(object_ptr_)
		, is_owning_(is_owning)
	{}

	std::string Name() override {
		return name_;
	}

	void* ObjectHandle() override {
		return dynamic_cast<void*>(&object_ptr);
	}

	void* PointedObjectHandle() override {
		return dynamic_cast<void*>(object_ptr);
	}

	bool IsOwning() override {
		return is_owning_;
	}

	virtual void SetValue(void* ptr) {
		object_ptr_ = static_cast<T*>(ptr);
	}

	std::shared_ptr<IVariable> PointedVariable() override {
		return serialize::CreateSerializerNode("object", *object_ptr_);
	}

protected:
	std::string name_;
	T*& object_ptr_;
	const bool is_owning_;
};

class IArrayVariable : IVariable {
public:
	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::Array;
	}

	virtual bool IsDynamicallyAllocated() = 0;
	virtual void* ArrayPointer() = 0;
	virtual void SetValues(void* array_ptr, std::size_t length) = 0;
	virtual std::size_t Length() = 0;
	virtual std::vector<std::shared_ptr<IVariable>> Elements() = 0;
};

template<class T>
class ArrayVariable : IArrayVariable {
public:
	ArrayVariable(std::string name, T* obj_array, std::size_t length, bool is_dynamically_allocated)
		: name_(name)
		, obj_array_(obj_array)
		, length_(length)
		, is_dynamically_allocated_(is_dynamically_allocated)
	{}

	std::string Name() override {
		return name_;
	}

	void* ObjectHandle() override {
		return dynamic_cast<void*>(&obj_array_);
	}

	void* ArrayPointer() override {
		return dynamic_cast<void*>(obj_array_);
	}

	void SetValues(void* array_ptr, std::size_t length) override {
		// Note: We cannot skip this and just do "new T(length)" when
		// allocating the array because T might just be a base/virtual
		// class. Therefore, we must allocate the correct derived class
		// elsewhere (using runtime type registry), and set the array
		// pointer and length here.
		obj_array_ = static_assert<T*>(array_ptr);
		length_ = length;
	}

	bool IsDynamicallyAllocated() override {
		return is_dynamically_allocated_;
	}

	std::size_t Length() override {
		return length_;
	}

	std::vector<std::shared_ptr<IVariable>> Elements() override {
		std::vector<std::shared_ptr<IVariable>> children(length_);
		for (std::size_t i = 0; i < length_; ++i) {
			std::stringstream element_name_ss;
			element_name_ss << "element_" << i;
			children[i] = serialize::CreateSerializerNode(element_name_ss.str(), obj_array_[i]);
		}
		return children;
	}

private:
	std::string name_;
	T* obj_array_;
	const std::size_t length_;
	const bool is_dynamically_allocated_;
};

class IClassVariable : IVariable {
public:
	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::Class;
	}

	virtual std::vector<std::shared_ptr<IVariable>> MemberVariables() = 0;

	virtual void OnWillSerializeMembers() {};

	virtual void OnDidDeserializeAllMembers() {};
};

template<class...> using void_t = void;

template<class, class = void>
struct has_serializable_members : std::false_type {};
template<class T>
struct has_serializable_members <T, void_t<decltype(std::declval<T>().SerializableMemberVariables())>> : std::true_type {};

template<typename T>
class ClassVariable : IClassVariable {
	template<class, class = void>
	struct implements_will_serialize_all_members : std::false_type {};
	struct implements_will_serialize_all_members <B, void_t<decltype(OnWillSerializeAllMembers())>> : std::true_type {};

	template<class, class = void>
	struct implements_did_deserialize_all_members : std::false_type {};
	struct implements_did_deserialize_all_members <B, void_t<decltype(OnDidDeserializeAllMembers())>> : std::true_type {};

public:
	ClassVariable(std::string name, T& object)
		: name_(name)
		, object_(object) {
		static_assert(std::has_serializable_members<T>, "T does not have any serializable member variables.");
	}

	std::string Name() override {
		return name_;
	}

	void* ObjectHandle() override {
		return dynamic_cast<void*>(&object);
	}

	std::vector<std::shared_ptr<IVariable>> MemberVariables() override {
		return object_.SerializableMemberVariables();
	}

	std::enable_if<implements_will_serialize_all_members<T>::value, void>
		OnWillSerializeAllMembers() override {
		object_.OnWillSerializeAllMembers();
	}

	std::enable_if<implements_did_deserialize_all_members<T>::value, void>
		OnDidDeserializeAllMembers() override {
		object_.OnDidDeserializeAllMembers();
	}

private:
	std::string name_;
	T& object_;
};

template<typename T, typename BuilderClass>
class ClassVariable : IClassVariable {
	template<class, class = void>
	struct deconstructs_to_parameters : std::false_type {};
	struct deconstructs_to_parameters <BuilderClass, void_t<decltype(OnDeconstructToParameters(std::declval<const T&>()))>> : std::true_type {};

	template<class, class = void>
	struct constructs_from_parameters : std::false_type {};
	struct constructs_from_parameters <BuilderClass, void_t<decltype(OnConstructFromParameters(std::declval<T&>()))>> : std::true_type {};
public:
	ClassVariable(std::string name, T& object)
		: name_(name)
		, object_(object) {
		static_assert(std::has_serializable_members<T>, "T does not have any serializable member variables.");
		static_assert(std::deconstructs_to_parameters<BuilderClass>, "B does not deconstruct T into parameter variables.");
		static_assert(std::constructs_from_parameters<BuilderClass>, "B does not construct T from parameter variables.");
	}

	std::string Name() override {
		return name_;
	}

	void* ObjectHandle() override {
		return dynamic_cast<void*>(&object);
	}

	virtual void OnWillSerializeAllMembers() override {
		var_builder_.OnDeconstructToParameters(object_);
	}

	std::vector<std::shared_ptr<IVariable>> MemberVariables() override {
		return var_builder_.SerializableMemberVariables();
	}

	virtual void OnDidDeserializeAllMembers() override {
		var_builder_.OnConstructFromParameters(object_);
	}

private:
	std::string name_;
	T& object_;
	BuilderClass var_builder_;
};
