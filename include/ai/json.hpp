#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cctype>

namespace ai {

enum class JsonType { Null, Bool, Number, String, Array, Object };

class Json {
public:
    JsonType type_{JsonType::Null};
    bool bool_val_{false};
    double num_val_{0.0};
    std::string str_val_;
    std::vector<Json> arr_val_;
    std::map<std::string, Json> obj_val_;

    Json() = default;
    Json(std::nullptr_t) : type_(JsonType::Null) {}
    Json(bool b) : type_(JsonType::Bool), bool_val_(b) {}
    Json(int n) : type_(JsonType::Number), num_val_(static_cast<double>(n)) {}
    Json(long n) : type_(JsonType::Number), num_val_(static_cast<double>(n)) {}
    Json(double n) : type_(JsonType::Number), num_val_(n) {}
    Json(const char* s) : type_(JsonType::String), str_val_(s ? s : "") {}
    Json(std::string s) : type_(JsonType::String), str_val_(std::move(s)) {}
    Json(JsonType t) : type_(t) {}

    static Json array() { return Json(JsonType::Array); }
    static Json object() { return Json(JsonType::Object); }

    bool is_null() const { return type_ == JsonType::Null; }
    bool is_bool() const { return type_ == JsonType::Bool; }
    bool is_number() const { return type_ == JsonType::Number; }
    bool is_string() const { return type_ == JsonType::String; }
    bool is_array() const { return type_ == JsonType::Array; }
    bool is_object() const { return type_ == JsonType::Object; }

    bool as_bool(bool def = false) const { return is_bool() ? bool_val_ : def; }
    double as_number(double def = 0.0) const { return is_number() ? num_val_ : def; }
    int as_int(int def = 0) const { return is_number() ? static_cast<int>(num_val_) : def; }
    const std::string& as_string() const { return str_val_; }
    std::string as_string(const std::string& def) const { return is_string() ? str_val_ : def; }

    void push_back(const Json& v) { if (type_ == JsonType::Array) arr_val_.push_back(v); }
    size_t size() const { return is_array() ? arr_val_.size() : (is_object() ? obj_val_.size() : 0); }
    bool has(const std::string& key) const { return is_object() && obj_val_.find(key) != obj_val_.end(); }

    Json& operator[](size_t idx) { return arr_val_.at(idx); }
    const Json& operator[](size_t idx) const { return arr_val_.at(idx); }

    Json& operator[](const std::string& key) {
        if (type_ != JsonType::Object) { type_ = JsonType::Object; }
        return obj_val_[key];
    }

    const Json& get(const std::string& key) const {
        static const Json kNull;
        if (!is_object()) return kNull;
        auto it = obj_val_.find(key);
        return it != obj_val_.end() ? it->second : kNull;
    }

    static std::string escape(const std::string& s);
    std::string dump(int indent = -1, int depth = 0) const;
    static Json parse(const std::string& src, std::string& err);
};

} // namespace ai
