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
	NonOwningPointerVariable,
	OwningPointerVariable,
	StaticArray,
	DynamicArray,
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

class INonOwningPointerVariable : IVariable {
public:
	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::NonOwningPointerVariable;
	}

	virtual void* PointedObjectHandle() = 0;
	virtual void SetValue(void* ptr) = 0;
};

template<typename T>
class NonOwningPointerVariable : INonOwningPointerVariable {
public:
	NonOwningPointerVariable(std::string name, T*& object_ptr)
		: name_(name)
		, object_ptr_(object_ptr_) {}

	std::string Name() {
		return name_;
	}

	void* ObjectHandle() {
		return &object_ptr;
	}

	void* PointedObjectHandle() override {
		return object_ptr;
	}

	virtual void SetValue(void* ptr) {
		object_ptr_ = static_cast<T*>(ptr);
	}

protected:
	std::string name_;
	T*& object_ptr_;
};

class IOwningPointerVariable : IVariable {
public:
	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::OwningPointerVariable;
	}

	virtual void* OwnedObjectHandle() = 0;
	virtual std::shared_ptr<IVariable> OwnedVariable() = 0;
	virtual void SetValue(void* ptr) = 0;
};

template<typename T>
class OwningPointerVariable : IOwningPointerVariable {
public:
	OwningPointerVariable(std::string name, T*& object_ptr)
		: name_(name)
		, object_ptr_(object_ptr_) {}

	std::string Name() {
		return name_;
	}

	void* ObjectHandle() {
		return &object_ptr;
	}

	void* OwnedObjectHandle() override {
		return object_ptr;
	}

	std::shared_ptr<IVariable> OwnedVariable() override {
		return serialize::CreateSerializerNode("object", *object_ptr_);
	}

	virtual void SetValue(void* ptr) {
		object_ptr_ = static_cast<T*>(ptr);
	}

protected:
	std::string name_;
	T*& object_ptr_;
};

class IStaticArrayVariable : IVariable {
public:
	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::StaticArray;
	}

	virtual std::size_t Length() = 0;
	virtual std::vector<std::shared_ptr<IVariable>> Elements() = 0;
};

template<class T, std::size_t N>
class StaticArrayVariable : IStaticArrayVariable {
public:
	StaticArrayVariable(std::string name, T(&obj_array)[N])
		: name_(name)
		, obj_array_(obj_array)
	{}

	std::string Name() override {
		return name_;
	}

	void* ObjectHandle() override {
		return obj_array_;
	}

	std::size_t Length() override {
		return N;
	}

	std::vector<std::shared_ptr<IVariable>> Elements() override {
		std::vector<std::shared_ptr<IVariable>> children(N);
		for (std::size_t i = 0; i < N; ++i) {
			std::stringstream element_name_ss;
			element_name_ss << "element_" << i;
			children[i] = serialize::CreateSerializerNode(element_name_ss.str(), obj_array_[i]);
		}
		return children;
	}

private:
	std::string name_;
	T(&obj_array_)[N];
};

class IDynamicArrayVariable : IVariable {
public:
	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::DynamicArray;
	}

	virtual std::size_t Count() = 0;
	virtual std::vector<std::shared_ptr<IVariable>> Elements() = 0;
};

template<class T>
class DynamicArrayVariable : IStaticArrayVariable {
public:
	DynamicArrayVariable(std::string name, T* obj_array, std::size_t count)
		: name_(name)
		, obj_array_(obj_array)
		, count_(count)
	{}

	std::string Name() override {
		return name_;
	}

	void* ObjectHandle() override {
		return obj_array_;
	}

	std::size_t Count() override {
		return N;
	}

	std::vector<std::shared_ptr<IVariable>> Elements() override {
		std::vector<std::shared_ptr<IVariable>> children(N);
		for (std::size_t i = 0; i < count_; ++i) {
			std::stringstream element_name_ss;
			element_name_ss << "element_" << i;
			children[i] = serialize::CreateSerializerNode(element_name_ss.str(), obj_array_[i]);
		}
		return children;
	}

private:
	std::string name_;
	T* obj_array_;
	std::size_t count_;
};

class IClassVariable : IVariable {
public:
	VariableTypeCategory TypeCategory() override {
		return VariableTypeCategory::Class;
	}

	virtual std::vector<std::shared_ptr<IVariable>> MemberVariables() = 0;
	virtual void OnWillSerializeMembers() = 0;
	virtual void OnDidDeserializeAllMembers() = 0;
};

template<class...> using void_t = void;

template<class, class = void>
struct has_serializable_members : std::false_type {};
template<class T>
struct has_serializable_members <T, void_t<decltype(std::declval<T>().SerializableMemberVariables())>> : std::true_type {};

template<typename T>
class ClassVariable : IClassVariable {
public:
	ClassVariable(std::string name, T& object)
		: name_(name)
		, object_(object) {
		static_assert(std::has_serializable_members<T>, "T does not have any serializable member variables.");
	}

	std::string Name() {
		return name_;
	}

	void* ObjectHandle() {
		return &object;
	}

	virtual void OnWillSerializeAllMembers() override {
		object_.OnWillSerializeAllMembers();
	}

	std::vector<std::shared_ptr<IVariable>> MemberVariables() override {
		return object_.SerializableMemberVariables();
	}

	virtual void OnDidDeserializeAllMembers() override {
		object_.OnDidDeserializeAllMembers();
	}

private:
	std::string name_;
	T& object_;
};

template<typename B, typename T>
class ClassVariable : IClassVariable {
private:
	template<class, class = void>
	struct deconstructs_to_parameters : std::false_type {};
	struct deconstructs_to_parameters <B, void_t<decltype(OnDeconstructToParameters(std::declval<const T&>()))>> : std::true_type {};

	template<class, class = void>
	struct constructs_from_parameters : std::false_type {};
	struct constructs_from_parameters <B, void_t<decltype(OnDeconstructToParameters(std::declval<const T&>()))>> : std::true_type {};
public:
	ClassVariable(std::string name, T& object)
		: name_(name)
		, object_(object) {
		static_assert(std::has_serializable_members<T>, "T does not have any serializable member variables.");
		static_assert(std::deconstructs_to_parameters<B>, "B does not deconstruct T into parameter variables.");
		static_assert(std::constructs_from_parameters<B>, "B does not construct T from parameter variables.");
		var_builder_ = new B();
	}

	~ClassVariable() {
		delete var_builder_;
	}

	std::string Name() {
		return name_;
	}

	void* ObjectHandle() {
		return &object;
	}

	virtual void OnWillSerializeAllMembers() override {
		var_builder_->OnDeconstructToParameters(object_);
	}

	std::vector<std::shared_ptr<IVariable>> MemberVariables() override {
		return var_builder_->SerializableMemberVariables();
	}

	virtual void OnDidDeserializeAllMembers() override {
		var_builder_->OnConstructFromParameters(object_);
	}

private:
	std::string name_;
	T& object_;
	B* var_builder_;
};
