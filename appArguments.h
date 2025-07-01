//
// Created by shtykov on 6/26/25.
//

#ifndef APP_ARGUMENTS_H
#define APP_ARGUMENTS_H
#include <string>
#include <map>
#include <utility>
#include <variant>
#include <optional>

namespace app::arguments {
    enum class ValueType {
        None = 0,
        String,
        Int32,
        Int32h, // hex 32-bit integer
        Int64,
        Int64h, // hex 64-bit integer
        Float,
    };

    typedef std::variant<std::string, int32_t, int64_t, double, bool> Value;
    typedef std::optional<Value> DefaultValue;
    enum class Type {
        None = 0,
        Optional,
        Mandatory,
    };
    class Options {
    public:
        std::string name;
        Type type;
        std::string desc;
        char shortCut;
        ValueType valueType;
        DefaultValue defaultValue;
        Options(const char sc, std::string name, std::string  descr, const Type mandatory, const ValueType valType, DefaultValue dv) :
            name(std::move(name)), type(mandatory), desc(std::move(descr)), shortCut(sc), valueType(valType), defaultValue(std::move(dv)) {
        };
        Options(const char sc, std::string name, std::string  descr, const Type mandatory, const ValueType valType) :
            name(std::move(name)), type(mandatory),desc(std::move(descr)), shortCut(sc), valueType(valType), defaultValue() {};
        Options(const char sc, std::string name, std::string  descr) :
            name(std::move(name)), type(Type::None),desc(std::move(descr)), shortCut(sc), valueType(ValueType::None), defaultValue() {};
        Value acquireValue(const char* pVal) const;
    };
    typedef std::map<std::string, Options> OptionsMap;
}

#endif //APP_ARGUMENTS_H