#pragma once

#include <iostream>
#include <type_traits>
#include <memory>
#include <unordered_map>

#include <core/utils/type_id_mapper.h>
#include <core/utils/set_trie.h>

#define REGISTER_MY_TYPE(typeName) int RegisterMyType() { \
        RuntimeTypeRegistry.GetInstance().RegisterType<std::remove_reference(decltype(*this))>(#typeName); \
    } \
static int i = RegisterMyType(); \

#define REGISTER_MY_TYPE(typeName, ...) int RegisterMyType() { \
        RuntimeTypeRegistry.GetInstance().RegisterTypeWithTemplateArgs<std::remove_reference(decltype(*this)), __VA_ARGS__>(#typeName); \
    } \
static int i = RegisterMyType(); \

// TODO: Put all arithmetic types here
#define FOREACH_ARITHMETIC_TYPE(ACTION) \
    ACTION(int) \
    ACTION(float)

#define FOREACH_STL_TYPE(ACTION) \
    ACTION(std::vector, T) \
    ACTION(std::unordered_map, K, V)

#define REGISTER_ARITHMETIC_TYPE(typeName) RegisterType<##typeName>(#typeName); \

#define REGISTER_STL_TYPE(typeName) template<typename T> \
const char* GetTypeName() { \
    int type_id = type_id_mapper_.GetTypeId<T>(); \
    return rt_type_data_map_[type_id].type_name; \
} \
#define REGISTER_STL_TYPE(typeName, ...) RegisterType<##typeName>(#typeName); \

namespace rtti {
    class RuntimeTypeRegistry {
    private:
        typedef void* (*DynamicInstantiator)();
        struct RuntimeTypeInfo {
            std::string type_name;
            DynamicInstantiator instantiator;
        };

        TypeIDMapper type_id_mapper_;
        std::unordered_map<int, RuntimeTypeInfo> rt_type_info_map_;
        SetTrie<char, int> type_name_to_id_trie_;

        template<typename T>
        static void* Instantiate() {
            return new T();
        }

        RuntimeTypeRegistry() {
            FOREACH_ARITHMETIC_TYPE(REGISTER_ARITHMETIC_TYPE)
        }

    public:

        static RuntimeTypeRegistry& GetInstance() {
            static RuntimeTypeRegistry rtReg;
            return rtReg;
        }

        void* InstantiateObjectFromTypeName(const char* type_name) {
            int* type_id;
            std::string type_name_string(type_name);
            std::vector<char> type_name_key_set(type_name_string.begin(), type_name_string.end());
            if (!type_name_to_id_trie_.TryGetValueForKeySet(type_name_key_set, type_id)) {
                return nullptr;
            }

            return rt_type_info_map_[*type_id].instantiator();
        }


        template<class...> using void_t = void;

        template<class, class = void>
        struct is_self_registering_type : std::false_type {};

        template<class T>
        struct is_self_registering_type <T, void_t<decltype(std::declval<int>().RegisterMyType())>> : std::true_type {};

        /*
        template<typename std::vector<T>>
        void RegisterType(const char* type_name) {
            const int type_id = type_id_mapper_.GetTypeId<T>();

            RuntimeTypeInfo rti;
            rti.type_name = std::string(type_name);
            rti.instantiator = &Instantiate<T>;
            rt_type_info_map_.insert({ type_id, rti });

            std::vector<char> type_name_key_set(rti.type_name.begin(), rti.type_name.end());
            type_name_to_id_trie_.InsertValueForKeySet(type_name_key_set, type_id);
        }

        template <typename T>
        std::enable_if<is_self_registering_type<T>::value, void>
            RegisterType(const char* type_name) {

        }
        */

        template<typename T>
        void RegisterType(const char* type_name) {
            const int type_id = type_id_mapper_.GetTypeId<T>();

            RuntimeTypeInfo rti;
            rti.type_name = std::string(type_name);
            rti.instantiator = &Instantiate<T>;
            rt_type_info_map_.insert({ type_id, rti });

            std::vector<char> type_name_key_set(rti.type_name.begin(), rti.type_name.end());
            type_name_to_id_trie_.InsertValueForKeySet(type_name_key_set, type_id);
        }

        template<typename T, typename ...Args>
        void RegisterTypeWithTemplateArgs(const char* type_name) {
            const std::size_t num_template_args = sizeof...(Args);
            const char* template_arg_type_names[num_template_args] = { GetTypeName<Args>()... };

            // Create new type name for this templated class.
            std::string aggregated_type_name(type_name);
            aggregated_type_name.append("<");
            for (std::size_t i = 0; i < num_template_args; ++i) {
                aggregated_type_name.append(template_arg_type_names[i]);
            }
            aggregated_type_name.append(">");

            RegisterType<T>(aggregated_type_name.c_str());
        }

        template<typename T>
        const char* GetTypeName() {
            int type_id = type_id_mapper_.GetTypeId<T>();
            return rt_type_info_map_[type_id].type_name;
        }

        template<typename K, typename V>
        const char* GetTypeName<std::unordered_map<K, V>>() {
            std::string type_name("std::unordered_map");
            type_name.append(GetTypeName<K>());
            type_name.append(GetTypeName<V>());
            return type_name;
        }
    };
}
