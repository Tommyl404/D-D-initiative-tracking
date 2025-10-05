#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace simplejson {

class JsonValue {
public:
    using Array = std::vector<JsonValue>;
    using Object = std::map<std::string, JsonValue>;

    JsonValue();
    explicit JsonValue(std::nullptr_t);
    explicit JsonValue(bool value);
    explicit JsonValue(int64_t value);
    explicit JsonValue(std::string value);
    JsonValue(const char* value);
    explicit JsonValue(Array value);
    explicit JsonValue(Object value);

    bool isNull() const;
    bool isBool() const;
    bool isNumber() const;
    bool isString() const;
    bool isArray() const;
    bool isObject() const;

    bool asBool() const;
    int64_t asInt() const;
    const std::string& asString() const;
    const Array& asArray() const;
    Array& asArray();
    const Object& asObject() const;
    Object& asObject();

    bool contains(const std::string& key) const;
    const JsonValue& at(const std::string& key) const;
    JsonValue& operator[](const std::string& key);

    const JsonValue& at(std::size_t index) const;

private:
    std::variant<std::nullptr_t, bool, int64_t, std::string, Array, Object> value_;
};

JsonValue parse(const std::string& text);
std::string stringify(const JsonValue& value, int indentSize = 2);

}
