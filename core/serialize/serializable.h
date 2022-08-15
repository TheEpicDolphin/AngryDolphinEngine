#pragma once

#include "variable.h"

#define ARITHMETIC(arithmeticVar) std::make_shared<ArithmeticVariable>(#arithmeticVar, ##arithmeticVar)
#define ENUM(enumVar) std::make_shared<EnumVariable>(#enumVar, ##enumVar)
#define NON_OWNING_PTR(NonOwningPtrVar) std::make_shared<NonOwningPointerVariable>(#NonOwningPtrVar, ##NonOwningPtrVar)
#define OWNING_PTR(owningPtrVar) std::make_shared<OwningPointerVariable>(#owningPtrVar, ##owningPtrVar)
#define STATIC_ARRAY(staticArrayVar) std::make_shared<StaticArrayVariable>(#staticArrayVar, ##staticArrayVar)
#define DYNAMIC_ARRAY(dynamicArrayVar, count) std::make_shared<DynamicArrayVariable>(#dynamicArrayVar, ##dynamicArrayVar, ##count)
#define CLASS(classVar) std::make_shared<ClassVariable>(#classVar, ##classVar)
#define CLASS(classVar, BuilderClass) std::make_shared<ClassVariable<##BuilderClass>>(#classVar, ##classVar)

#define SERIALIZABLE_MEMBERS(...) std::vector<std::shared_ptr<IVariable>> SerializableMemberVariables(){ \
    return { __VA_ARGS__ }; \
} \
