#pragma once

#include <iostream>
#include <type_traits>
#include <memory>
#include <unordered_map>

#include <core/utils/type_id_mapper.h>
#include <core/utils/set_trie.h>

#include "variable.h"

#define SERIALIZABLE_MEMBERS(...) std::vector<std::shared_ptr<IVariable>> SerializableMemberVariables(){ \
    return serializable::CreateVariables(#__VA_ARGS__, __VA_ARGS__); \
} \

#define REGISTER_TYPE(typeName, ...) struct TypeRegistrationStaticBlock { \
    TypeRegistrationStaticBlock() { \
        rt_type_registry.RegisterTypeWithTemplateArgs<##typeName, __VA_ARGS__>(#typeName); \
    } \
}; \
static TypeRegistrationStaticBlock ##typeName<__VA_ARGS__>::type_reg_; \

// TODO: Put all arithmetic types here
#define FOREACH_ARITHMETIC_TYPE(ACTION) \
    ACTION(int) \
    ACTION(float)

#define REGISTER_TYPE_WITHOUT_STATIC_BLOCK(typeName) RegisterType<##typeName>(#typeName); \

namespace serialize {
    class RuntimeTypeRegistry {
    private:
        typedef void* (*DynamicInstantiator)();
        struct RuntimeTypeData {
            const char* type_name;
            DynamicInstantiator instantiator;
        };

        TypeIDMapper type_id_mapper_;
        std::unordered_map<int, RuntimeTypeData> rt_type_data_map_;
        SetTrie<char, int> type_name_to_id_trie_;

        template<typename T>
        static void* Instantiate() {
            return new T();
        }

    public:
        RuntimeTypeRegistry() {
            FOREACH_ARITHMETIC_TYPE(REGISTER_TYPE_WITHOUT_STATIC_BLOCK)
        }

        void* InstantiateObjectFromTypeName(const char* type_name) {
            int* type_id;
            if (!type_name_to_id_trie_.TryGetValueForKeySet(type_name, type_id)) {
                return nullptr;
            }

            return rt_type_data_map_[*type_id].instantiator();
        }

        template<typename T>
        void RegisterType(const char* type_name) {
            const int type_id = type_id_mapper_.GetTypeId<T>();
            rt_type_data_map_[type_id] = { type_name, &Instantiate<T> };
            type_name_to_id_trie_.InsertValueForKeySet(type_name, type_id);
        }

        template<typename T, typename ...Args>
        void RegisterTypeWithTemplateArgs(const char* type_name) {
            const int type_id = type_id_mapper_.GetTypeId<T>();
            const char* template_arg_type_names[sizeof(Args)] = { GetTypeName<Args>()... };
            // TODO: implement
            const char* aggregated_type_name = ;
            rt_type_data_map_[type_id] = { aggregated_type_name, &Instantiate<T> };
            type_name_to_id_trie_.InsertValueForKeySet(aggregated_type_name, type_id);
        }

        template<typename T>
        const char* GetTypeName() {
            int type_id = type_id_mapper_.GetTypeId<T>();
            return rt_type_data_map_[type_id].type_name;
        }
    };
    static RuntimeTypeRegistry rt_type_registry;

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
    std::enable_if<std::is_pointer<T>::value, std::shared_ptr<IVariable>>
        CreateVariable(std::string name, T& pointer_object) {
        return std::make_shared<PointerVariable<T>>(name, pointer_object);
    }

    template <typename T, std::size_t N>
    std::shared_ptr<IVariable> CreateVariable(std::string name, T(&array_object)[N]) {
        return std::make_shared<ArrayVariable<T, N>>(name, array_object);
    }

    template<class...> using void_t = void;

    template<class, class = void>
    struct has_serializable_members : std::false_type {};

    template<class T>
    struct has_serializable_members <T, void_t<decltype(std::declval<T>().SerializableMemberVariables())>> : std::true_type {};

    template <typename T>
    std::enable_if<has_serializable_members<T>::value, std::shared_ptr<IVariable>>
    CreateVariable(std::string name, T& class_object) {
        return std::make_shared<ClassVariable<T>>(name, class_object);
    }

    template <typename... Args>
    std::vector<std::shared_ptr<IVariable>> CreateVariables(const char* comma_separated_names, Args&... args) {
        const std::size_t member_count = sizeof...(Args);
        std::string names[member_count];
        const char* arg_name_location = comma_separated_names;
        for (int i = 0; i < member_count; ++i) {
            const char* comma_location = strchr(arg_name_location, ',');
            if (comma_location == nullptr) {
                names[i] = std::string(arg_name_location);
            } else {
                names[i] = std::string(arg_name_location, comma_location - arg_name_location);
                arg_name_location = comma_location + 1;
                // Skip whitespace
                while ((*arg_name_location != '\0') && std::isspace(*arg_name_location) > 0) {
                    arg_name_location++;
                }
            }
        }

        int child_index = 0;
        return { (CreateVariable(names[child_index++], args))... };
    }
}
