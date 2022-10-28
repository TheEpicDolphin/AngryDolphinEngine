#pragma once

#include <iostream>
#include <type_traits>
#include <memory>
#include <unordered_map>

#include <core/utils/type_id_mapper.h>
#include <core/utils/set_trie.h>

#define TYPE_NAME(typeName) std::string TypeName() { \
        return std::string(#typeName); \
    } \

#define TEMPLATE_TYPE_NAME(typeName, ...) std::string TypeName() { \
        return std::string(#typeName) + TemplateArgsAsString<__VA_ARGS__>(); \
    } \

// TODO: Put all arithmetic types here
#define FOREACH_ARITHMETIC_TYPE(ACTION) \
    ACTION(int) \
    ACTION(float)

#define FOREACH_STL_TYPE(ACTION) \
    ACTION(std::vector, T) \
    ACTION(std::unordered_map, K, V)

#define ASSIGN_TYPE_NAME(typeName) arithmetic_type_name_map_[type_id_mapper_.GetTypeId<typeName>()] = #typeName; \

#define REGISTER_STL_TYPE(typeName) template<typename T> \
const char* GetTypeName() { \
    int type_id = type_id_mapper_.GetTypeId<T>(); \
    return initial_type_name_map_[type_id].type_name; \
} \

#define REGISTER_STL_TYPE(typeName, ...) RegisterType<##typeName>(#typeName); \

namespace rtti {
    class RuntimeTypeRegistry {
    private:
        typedef void* (*NewObjectConstructor)();
        typedef void* (*NewArrayConstructor)(std::size_t);

        template <typename T>
        class ObjectTypeNameConstructorRegistrar {
        private:
            static void* NewObject() { return new T(); }
        public:
            static int type_id;

            static int RegisterObjectConstructor() {
                RuntimeTypeRegistry::GetInstance().RegisterObjectConstructor<T>(&NewObject);
            }
        };

        template <typename T>
        static int ObjectTypeNameConstructorRegistrar<T>::type_id = ObjectTypeNameConstructorRegistrar<T>::RegisterObjectConstructor();

        // ===========================================================================

        template <typename T>
        class ArrayTypeNameConstructorRegistrar {
        private:
            static void* NewArray(std::size_t length) { return new T[length]; }
        public:
            static int type_id;

            static int RegisterArrayConstructor() {
                RuntimeTypeRegistry::GetInstance().RegisterArrayConstructor<T>(&NewArray);
            }
        };

        template <typename T>
        static int ArrayTypeNameConstructorRegistrar<T>::type_id = ArrayTypeNameConstructorRegistrar<T>::RegisterArrayConstructor();

        // ===========================================================================
        typedef void* (*TypeUpcaster)(void*);

        template <typename _Derived, typename _Base>
        class TypeUpcasterRegistrar {
        private:
            static void* Upcast(void* from) {
                _Derived* from = static_cast<_Derived*>(from);
                // Implicit upcast.
                _Base* to = from;
                return to;
            }

        public:
            static int type_id;

            static int RegisterTypeUpcaster() {
                RuntimeTypeRegistry::GetInstance().RegisterTypeUpcaster<_Derived, _Base>(derived_type_id, base_type_id, &Upcast);
            }
        };

        template <typename _Derived, typename _Base>
        static int TypeUpcasterRegistrar<_Derived, _Base>::type_id = TypeUpcasterRegistrar<_Derived, _Base>::RegisterTypeUpcaster();

        // ===========================================================================
        typedef std::shared_ptr<IVariable> (*VariableConstructor)(void*);

        template <typename T>
        class VariableConstructorRegistrar {
        private:
            static std::shared_ptr<IVariable> CreateVariable(void* ptr) {
                T* concrete_ptr = static_cast<T*>(ptr);
                return serialize::CreateVariable("pointed_object", *concrete_ptr);
            }

        public:
            static int type_id;

            static int RegisterVariableConstructor() {
                RuntimeTypeRegistry::GetInstance().RegisterVariableConstructor<T>(&CreateVariable);
            }
        };

        template <typename T>
        static int VariableConstructorRegistrar<T>::type_id = VariableConstructorRegistrar<T>::RegisterVariableConstructor();

        // ===========================================================================

        TypeIDMapper type_id_mapper_;
        SetTrie<char, int> type_name_to_id_trie_;

        std::unordered_map<int, const char*> arithmetic_type_name_map_;
        std::unordered_map<int, NewObjectConstructor> new_object_constructor_map_;
        std::unordered_map<int, NewArrayConstructor> new_array_constructor_map_;
        std::unordered_map<int, VariableConstructor> variable_constructor_map_;
        std::unordered_map<int, ArrayVariableConstructor> array_variable_constructor_map_;
        // Maps type
        std::unordered_map<int, std::vector<std::pair<int, TypeUpcaster>>> type_upcaster_map_;

        RuntimeTypeRegistry() {
            FOREACH_ARITHMETIC_TYPE(ASSIGN_TYPE_NAME)
        }

        template <typename T>
        void MapTypeNameToId() {
            std::string type_name = GetTypeName<T>();
            std::vector<char> type_name_key_set(type_name.begin(), type_name.end());
            type_name_to_id_trie_.InsertValueForKeySet(type_name_key_set, GetTypeId<T>());
        }

        template <typename T>
        void RegisterObjectConstructor(
            NewObjectConstructor constructor
        ) {
            int type_id = GetTypeId<T>();
            new_object_constructor_map_[type_id] = constructor;
        }

        template <typename T>
        void RegisterArrayConstructor(
            NewArrayConstructor constructor
        ) {
            int type_id = GetTypeId<T>();
            new_object_constructor_map_[type_id] = constructor;
        }

        template <typename _Derived, typename _Base>
        void RegisterTypeUpcaster(
            TypeUpcaster upcaster
        ) {
            const int derived_type_id = GetTypeId<_Derived>();
            const int base_type_id = GetTypeId<_Base>();
            std::vector<char> type_name_key_set(type_name.begin(), type_name.end());
            new_object_for_type_name_trie_.InsertValueForKeySet(type_name_key_set, constructor);
        }

    public:
        static RuntimeTypeRegistry& GetInstance() {
            static RuntimeTypeRegistry rtReg;
            return rtReg;
        }

        template<typename T>
        const int GetTypeId() {
            return type_id_mapper_<T>.GetTypeId();
        }

        template<class...> using void_t = void;
        template<class, class = void>
        struct has_type_name : std::false_type {};
        template<class T>
        struct has_type_name <T, void_t<decltype(std::declval<std::string>().TypeName())>> : std::true_type {};

        template<typename T>
        const std::string GetTypeName() {
            int type_id = type_id_mapper_<T>.GetTypeId();
            auto iter = arithmetic_type_name_map_.find(type_id);
            if (iter != arithmetic_type_name_map_.end()) {
                return std::string(iter->second);
            }
            else {
                return T::TypeName();
            }
        }

        template<typename ...Args>
        const std::string TemplateArgsAsString() {
            const std::size_t num_template_args = sizeof...(Args);
            const std::string template_arg_type_names[num_template_args] = { GetTypeName<Args>()... };

            std::string template_args_string;
            template_args_string.append("<");
            for (std::size_t i = 0; i < num_template_args; ++i) {
                template_args_string.append(template_arg_type_names[i]);
                template_args_string.append(",");
            }
            template_args_string.append(">");
            return template_args_string;
        }

        template<typename K, typename V>
        const char* GetTypeName<std::unordered_map<K, V>>() {
            std::string type_name("std::unordered_map");
            type_name.append(GetTypeName<K>());
            type_name.append(GetTypeName<V>());
            return type_name;
        }

        void* NewObjectForTypeName(std::string type_name) {
            int* type_id;
            std::vector<char> type_name_key_set(type_name.begin(), type_name.end());
            if (!type_name_to_id_trie_.TryGetValueForKeySet(type_name_key_set, type_id)) {
                return nullptr;
            }

            return new_object_constructor_map_[*type_id]();
        }

        void* NewArrayForTypeName(std::string type_name, std::size_t length) {
            int* type_id;
            std::vector<char> type_name_key_set(type_name.begin(), type_name.end());
            if (!type_name_to_id_trie_.TryGetValueForKeySet(type_name_key_set, type_id)) {
                return nullptr;
            }

            return new_array_constructor_map_[*type_id](length);
        }

        template<typename T>
        std::vector<void*> GetBasePointers(T* ptr) {

        }

        void* Upcast(std::string derived_type_name, void* from, std::string base_type_name) {

        }

        template<typename _Base>
        _Base* Upcast(std::string derived_type_name, void* derived_ptr) {

        }

        template<typename _Base>
        _Base* Upcast(int derived_type_id, void* derived_ptr) {

        }

        template<typename _Derived>
        void* Upcast(_Derived* derived_ptr, std::string base_type_name) {

        }

        std::shared_ptr<IVariable> GetPointedVariable(int derived_type_id, void* derived_ptr) {
            auto var_constructor_map_iter = variable_constructor_map_.find(derived_type_id);
            if (var_constructor_map_iter != variable_constructor_map_.end()) {
                return var_constructor_map_iter->second(derived_ptr);
            }
            
            auto array_var_constructor_map_iter = array_variable_constructor_map_.find(derived_type_id);
            if (array_var_constructor_map_iter != array_variable_constructor_map_.end()) {
                return array_var_constructor_map_iter->second(derived_ptr);
            }

            return nullptr;
        }
    };
}
