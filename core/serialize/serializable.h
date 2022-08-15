#pragma once

#include <iostream>
#include <type_traits>
#include <memory>
#include <unordered_map>
#include <vector>

#include "variable.h"

#define ARITHMETIC(arithmeticVar) std::make_shared<ArithmeticVariable>(#arithmeticVar, ##arithmeticVar)
#define ENUM(enumVar) std::make_shared<EnumVariable>(#enumVar, ##enumVar)
#define NON_OWNING_PTR(NonOwningPtrVar) std::make_shared<NonOwningPointerVariable>(#NonOwningPtrVar, ##NonOwningPtrVar)
#define OWNING_PTR(owningPtrVar) std::make_shared<OwningPointerVariable>(#owningPtrVar, ##owningPtrVar)
#define STATIC_ARRAY(staticArrayVar) std::make_shared<StaticArrayVariable>(#staticArrayVar, ##staticArrayVar)
#define DYNAMIC_ARRAY(dynamicArrayVar, count) std::make_shared<DynamicArrayVariable>(#dynamicArrayVar, ##dynamicArrayVar, ##count)
#define CLASS(classVar) std::make_shared<ClassVariable>(#classVar, ##classVar)
#define CLASS(classVar, builderClass) std::make_shared<ClassVariable<##builderClass>>(#classVar, ##classVar)

#define SERIALIZABLE_MEMBERS(...) std::vector<std::shared_ptr<IVariable>> SerializableMemberVariables(){ \
    return { __VA_ARGS__ }; \
} \

namespace serialize {
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
