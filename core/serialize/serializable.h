#pragma once

#include "variable.h"

#define VAR(var) serialize::CreateVariable(#var, ##var)
#define VAR_BUILDER(var, BuilderClass) serialize::CreateVariable<##BuilderClass>(#var, ##var)

#define SERIALIZABLE_MEMBERS(...) std::vector<std::shared_ptr<IVariable>> SerializableMemberVariables(){ \
    return { __VA_ARGS__ }; \
} \

template <typename T, typename... Args>
static dm_object_pointer<T> make_object(Args... args) { return dm_object_pointer(new T(args)); }

template <typename T>
class dm_object_pointer {
public:
    dynamic_memory_object() {}
    dynamic_memory_object(T* object_ptr) { object_ptr_ = object_ptr; }

    T& operator* () { return *object_ptr_; }
    T* operator-> () { return object_ptr_; }

    T* get() { return object_ptr_ }

    void reset(T* object_ptr) { object_ptr_ = object_ptr }
    void destroy() { delete object_ptr_; }
    
private:
    T* object_ptr_;
};

template <typename T>
class dm_array {
public:
    dm_array() {}

    dm_array(T* array_ptr, std::size_t length) {
        array_ptr_ = array_ptr;
        length_ = length;
    }

    dm_array(std::size_t length) {
        array_ptr_ = new T[length];
        length_ = length;
    }

    void reset(T* array_ptr, std::size_t length) {
        array_ptr_ = array_ptr;
        length_ = length;
    }

    T* begin() { return array_ptr_; }
    T* end() { return array_ptr_ + length_; }
    std::size_t length() { return length_; }

    int operator [] (int i) const { return array_ptr_[i]; }
    int& operator [] (int i) { return array_ptr_[i]; }

    void destroy() { delete[] array_ptr_; }

private:
    T* array_ptr_;
    std::size_t length_;
};

namespace serialize {

    template <typename T>
    std::enable_if<std::is_arithmetic<T>::value, std::shared_ptr<IVariable>>
        CreateVariable(std::string name, T& arithmetic_object) {
        return std::make_shared<ArithmeticVariable<T>>(name, arithmetic_object);
    }

    template <typename T>
    std::enable_if<std::is_enum<T>::value, std::shared_ptr<IVariable>>
        CreateVariable(std::string name, T& enum_object) {
        return std::make_shared<EnumVariable<T>>(name, enum_object);
    }

    template <typename T>
    std::shared_ptr<IVariable> CreateVariable(std::string name, T* pointer) {
        return std::make_shared<PointerVariable<T>>(name, pointer, false);
    }

    template <typename T>
    std::shared_ptr<IVariable> CreateVariable(std::string name, dm_object_pointer<T>& dynamic_object) {
        return std::make_shared<PointerVariable<T>>(name, dynamic_object.get(), true);
    }

    template <typename T, std::size_t N>
    std::shared_ptr<IVariable> CreateVariable(std::string name, T(&static_array)[N]) {
        return std::make_shared<ArrayVariable<T>>(name, static_array, N);
    }

    template <typename T>
    std::shared_ptr<IVariable> CreateVariable(std::string name, dm_array<T>& dynamic_array) {
        return std::make_shared<ArrayVariable<T>>(name, dynamic_array.begin(), dynamic_array.length());
    }

    template <typename T>
    std::enable_if<has_serializable_members<T>::value, std::shared_ptr<IVariable>>
        CreateVariable(std::string name, T& class_object) {
        return std::make_shared<ClassVariable<T>>(name, class_object);
    }
    
    template <typename BuilderClass, typename T>
    std::enable_if<has_serializable_members<T>::value, std::shared_ptr<IVariable>>
        CreateVariable(std::string name, T& class_object) {
        return std::make_shared<ClassVariable<T, BuilderClass>>(name, class_object);
    }
}
