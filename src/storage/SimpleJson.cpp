#include "storage/SimpleJson.h"

#include <charconv>
#include <cctype>
#include <sstream>

namespace simplejson {

namespace {
class Parser {
public:
    explicit Parser(std::string_view text)
        : text_(text) {}

    JsonValue parse() {
        skipWhitespace();
        JsonValue value = parseValue();
        skipWhitespace();
        if (!isAtEnd()) {
            throw std::runtime_error("Unexpected trailing characters in JSON");
        }
        return value;
    }

private:
    JsonValue parseValue() {
        if (isAtEnd()) {
            throw std::runtime_error("Unexpected end of JSON");
        }
        char ch = peek();
        if (ch == '"') {
            return JsonValue(parseString());
        }
        if (ch == '{') {
            return JsonValue(parseObject());
        }
        if (ch == '[') {
            return JsonValue(parseArray());
        }
        if (ch == 't') {
            consumeLiteral("true");
            return JsonValue(true);
        }
        if (ch == 'f') {
            consumeLiteral("false");
            return JsonValue(false);
        }
        if (ch == 'n') {
            consumeLiteral("null");
            return JsonValue(nullptr);
        }
        if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch))) {
            return JsonValue(parseNumber());
        }
        throw std::runtime_error("Invalid JSON token");
    }

    JsonValue::Object parseObject() {
        expect('{');
        skipWhitespace();
        JsonValue::Object object;
        if (match('}')) {
            return object;
        }
        while (true) {
            skipWhitespace();
            std::string key = parseString();
            skipWhitespace();
            expect(':');
            skipWhitespace();
            object.emplace(std::move(key), parseValue());
            skipWhitespace();
            if (match('}')) {
                break;
            }
            expect(',');
            skipWhitespace();
        }
        return object;
    }

    JsonValue::Array parseArray() {
        expect('[');
        skipWhitespace();
        JsonValue::Array array;
        if (match(']')) {
            return array;
        }
        while (true) {
            skipWhitespace();
            array.push_back(parseValue());
            skipWhitespace();
            if (match(']')) {
                break;
            }
            expect(',');
            skipWhitespace();
        }
        return array;
    }

    std::string parseString() {
        expect('"');
        std::string result;
        while (!isAtEnd()) {
            char ch = advance();
            if (ch == '"') {
                return result;
            }
            if (ch == '\\') {
                if (isAtEnd()) {
                    throw std::runtime_error("Invalid escape sequence");
                }
                char escape = advance();
                switch (escape) {
                    case '"': result.push_back('"'); break;
                    case '\\': result.push_back('\\'); break;
                    case '/': result.push_back('/'); break;
                    case 'b': result.push_back('\b'); break;
                    case 'f': result.push_back('\f'); break;
                    case 'n': result.push_back('\n'); break;
                    case 'r': result.push_back('\r'); break;
                    case 't': result.push_back('\t'); break;
                    default:
                        throw std::runtime_error("Unsupported escape sequence");
                }
            } else {
                result.push_back(ch);
            }
        }
        throw std::runtime_error("Unterminated string literal");
    }

    int64_t parseNumber() {
        std::size_t start = index_;
        if (match('-')) {
            // allow negative numbers
        }
        if (match('0')) {
            // leading zero allowed only for zero
        } else {
            while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
                advance();
            }
        }
        std::string_view slice = text_.substr(start, index_ - start);
        int64_t value = 0;
        auto [ptr, ec] = std::from_chars(slice.data(), slice.data() + slice.size(), value);
        if (ec != std::errc()) {
            throw std::runtime_error("Invalid numeric value in JSON");
        }
        return value;
    }

    void consumeLiteral(std::string_view literal) {
        for (char expected : literal) {
            if (isAtEnd() || advance() != expected) {
                throw std::runtime_error("Invalid JSON literal");
            }
        }
    }

    void skipWhitespace() {
        while (!isAtEnd()) {
            char ch = peek();
            if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
                advance();
            } else {
                break;
            }
        }
    }

    void expect(char expected) {
        if (isAtEnd() || advance() != expected) {
            throw std::runtime_error("Unexpected character in JSON");
        }
    }

    bool match(char expected) {
        if (isAtEnd() || text_[index_] != expected) {
            return false;
        }
        ++index_;
        return true;
    }

    char peek() const {
        return text_[index_];
    }

    char advance() {
        return text_[index_++];
    }

    bool isAtEnd() const {
        return index_ >= text_.size();
    }

    std::string_view text_;
    std::size_t index_{0};
};

std::string escapeString(const std::string& value) {
    std::string result;
    for (char ch : value) {
        switch (ch) {
            case '\\': result += "\\\\"; break;
            case '\"': result += "\\\""; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result.push_back(ch); break;
        }
    }
    return result;
}

std::string stringifyImpl(const JsonValue& value, int indentSize, int depth) {
    const std::string indent(depth * indentSize, ' ');
    const std::string childIndent((depth + 1) * indentSize, ' ');

    if (value.isNull()) {
        return "null";
    }
    if (value.isBool()) {
        return value.asBool() ? "true" : "false";
    }
    if (value.isNumber()) {
        return std::to_string(value.asInt());
    }
    if (value.isString()) {
        return '"' + escapeString(value.asString()) + '"';
    }
    if (value.isArray()) {
        const auto& array = value.asArray();
        if (array.empty()) {
            return "[]";
        }
        std::ostringstream oss;
        oss << "[\n";
        for (std::size_t i = 0; i < array.size(); ++i) {
            oss << childIndent << stringifyImpl(array[i], indentSize, depth + 1);
            if (i + 1 < array.size()) {
                oss << ",\n";
            } else {
                oss << "\n";
            }
        }
        oss << indent << "]";
        return oss.str();
    }

    const auto& object = value.asObject();
    if (object.empty()) {
        return "{}";
    }
    std::ostringstream oss;
    oss << "{\n";
    std::size_t index = 0;
    for (const auto& [key, child] : object) {
        oss << childIndent << "\"" << escapeString(key) << "\": "
            << stringifyImpl(child, indentSize, depth + 1);
        if (index + 1 < object.size()) {
            oss << ",\n";
        } else {
            oss << "\n";
        }
        ++index;
    }
    oss << indent << "}";
    return oss.str();
}
}

JsonValue::JsonValue()
    : value_(nullptr) {}

JsonValue::JsonValue(std::nullptr_t)
    : value_(nullptr) {}

JsonValue::JsonValue(bool value)
    : value_(value) {}

JsonValue::JsonValue(int64_t value)
    : value_(value) {}

JsonValue::JsonValue(std::string value)
    : value_(std::move(value)) {}

JsonValue::JsonValue(const char* value)
    : value_(std::string(value)) {}

JsonValue::JsonValue(Array value)
    : value_(std::move(value)) {}

JsonValue::JsonValue(Object value)
    : value_(std::move(value)) {}

bool JsonValue::isNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool JsonValue::isBool() const {
    return std::holds_alternative<bool>(value_);
}

bool JsonValue::isNumber() const {
    return std::holds_alternative<int64_t>(value_);
}

bool JsonValue::isString() const {
    return std::holds_alternative<std::string>(value_);
}

bool JsonValue::isArray() const {
    return std::holds_alternative<Array>(value_);
}

bool JsonValue::isObject() const {
    return std::holds_alternative<Object>(value_);
}

bool JsonValue::asBool() const {
    if (!isBool()) {
        throw std::runtime_error("JSON value is not a boolean");
    }
    return std::get<bool>(value_);
}

int64_t JsonValue::asInt() const {
    if (!isNumber()) {
        throw std::runtime_error("JSON value is not a number");
    }
    return std::get<int64_t>(value_);
}

const std::string& JsonValue::asString() const {
    if (!isString()) {
        throw std::runtime_error("JSON value is not a string");
    }
    return std::get<std::string>(value_);
}

const JsonValue::Array& JsonValue::asArray() const {
    if (!isArray()) {
        throw std::runtime_error("JSON value is not an array");
    }
    return std::get<Array>(value_);
}

JsonValue::Array& JsonValue::asArray() {
    if (!isArray()) {
        throw std::runtime_error("JSON value is not an array");
    }
    return std::get<Array>(value_);
}

const JsonValue::Object& JsonValue::asObject() const {
    if (!isObject()) {
        throw std::runtime_error("JSON value is not an object");
    }
    return std::get<Object>(value_);
}

JsonValue::Object& JsonValue::asObject() {
    if (!isObject()) {
        throw std::runtime_error("JSON value is not an object");
    }
    return std::get<Object>(value_);
}

bool JsonValue::contains(const std::string& key) const {
    if (!isObject()) {
        return false;
    }
    const auto& object = std::get<Object>(value_);
    return object.find(key) != object.end();
}

const JsonValue& JsonValue::at(const std::string& key) const {
    if (!isObject()) {
        throw std::runtime_error("JSON value is not an object");
    }
    const auto& object = std::get<Object>(value_);
    if (auto it = object.find(key); it != object.end()) {
        return it->second;
    }
    throw std::runtime_error("JSON key not found: " + key);
}

JsonValue& JsonValue::operator[](const std::string& key) {
    if (!isObject()) {
        value_ = Object{};
    }
    auto& object = std::get<Object>(value_);
    return object[key];
}

const JsonValue& JsonValue::at(std::size_t index) const {
    if (!isArray()) {
        throw std::runtime_error("JSON value is not an array");
    }
    const auto& array = std::get<Array>(value_);
    if (index >= array.size()) {
        throw std::runtime_error("JSON array index out of range");
    }
    return array[index];
}

JsonValue parse(const std::string& text) {
    Parser parser(text);
    return parser.parse();
}

std::string stringify(const JsonValue& value, int indentSize) {
    return stringifyImpl(value, indentSize, 0);
}

}
