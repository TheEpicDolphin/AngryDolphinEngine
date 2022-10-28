#pragma once

#include <core/utils/set_trie.h>

#include "variable.h"
/*
#define VAR(var) serialize::CreateVariable(#var, ##var)
#define VAR_BUILDER(var, BuilderClass) serialize::CreateVariable<##BuilderClass>(#var, ##var)

#define SERIALIZABLE_MEMBERS(...) std::vector<std::shared_ptr<IVariable>> SerializableMemberVariables(){ \
    return { __VA_ARGS__ }; \
} \

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
        return std::make_shared<PointerVariable<T>>(name, pointer);
    }

    template <typename T, std::size_t N>
    std::shared_ptr<IVariable> CreateVariable(std::string name, T(&static_array)[N]) {
        return std::make_shared<ArrayVariable<T>>(name, static_array, N, false);
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

    template<typename T, typename... Args>
    T* new_object(Args&... args) {
        T* object = new T(args);
        variable_map_[dynamic_cast<void*>(object)] = serialize::CreateVariable("pointed_object", object);
        return object;
    }

    template<typename T>
    T* new_array(std::size_t length) {
        T* array = new T[length];
        variable_map_[dynamic_cast<void*>(array)] = std::make_shared<ArrayVariable<T>>("pointed_array", array, length, true);
        return array;
    }

    template<typename T, typename... Args>
    void delete_object(T* object) {
        variable_map_.erase(dynamic_cast<void*>(object));
        delete object;
    }

    template<typename T>
    void delete_array(T* array) {
        variable_map_.erase(dynamic_cast<void*>(array));
        delete[] array;
    }

    template<typename _Derived, typename _Base>
    _Base* upcast(_Derived* from) {

    }

    std::shared_ptr<IVariable> GetPointedVariable(void* pointer) {
        auto iter = variable_map_.find(pointer);
        return iter == variable_map_.end() ? nullptr : iter->second;
    }
}
*/