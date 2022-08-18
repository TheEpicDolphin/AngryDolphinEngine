#pragma once

#include "variable.h"

#define VAR(var) serialize::CreateVariable(#var, ##var)
#define VAR(var, BuilderClass) serialize::CreateVariable<##BuilderClass>(#var, ##var)

#define SERIALIZABLE_MEMBERS(...) std::vector<std::shared_ptr<IVariable>> SerializableMemberVariables(){ \
    return { __VA_ARGS__ }; \
} \

template <typename T>
struct dynamic_memory_object {
    T* object_ptr;
};

template <typename T>
struct dynamic_memory_array {
    T* array_ptr;
    std::size_t length;

    T* begin() {
        return array_ptr;
    }

    T* end() {
        return array_ptr + length;
    }
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
        return std::make_shared<NonOwningPointerVariable<T>>(name, pointer);
    }

    template <typename T>
    std::shared_ptr<IVariable> CreateVariable(std::string name, dynamic_memory_object<T>& dynamic_object) {
        return std::make_shared<OwningPointerVariable<T>>(name, dynamic_object.object_ptr);
    }

    template <typename T, std::size_t N>
    std::shared_ptr<IVariable> CreateVariable(std::string name, T(&static_array)[N]) {
        return std::make_shared<StaticArrayVariable<T, N>>(name, static_array);
    }

    template <typename T>
    std::shared_ptr<IVariable> CreateVariable(std::string name, dynamic_memory_array<T>& dynamic_array) {
        return std::make_shared<DynamicArrayVariable<T>>(name, dynamic_array.array_ptr, dynamic_array.length);
    }

    template <typename T>
    std::enable_if<has_serializable_members<T>::value, std::shared_ptr<IVariable>>
        CreateVariable(std::string name, T& class_object) {
        return std::make_shared<ClassVariable<T>>(name, class_object);
    }

    template <typename BuilderClass, typename T>
    std::enable_if<has_serializable_members<T>::value, std::shared_ptr<IVariable>>
        CreateVariable(std::string name, T& class_object) {
        return std::make_shared<ClassVariable<BuilderClass, T>>(name, class_object);
    }
}
