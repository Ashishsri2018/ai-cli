#include "ai/json.hpp"
#include <limits>
#include <cstdio>

namespace ai {

std::string Json::escape(const std::string& s) {
    std::string out = "\"";
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += c;
                }
                break;
        }
    }
    out += "\"";
    return out;
}

std::string Json::dump(int indent, int depth) const {
    std::string ind = (indent >= 0) ? std::string(indent * depth, ' ') : "";
    std::string ind_next = (indent >= 0) ? std::string(indent * (depth + 1), ' ') : "";
    std::string nl = (indent >= 0) ? "\n" : "";
    std::string sp = (indent >= 0) ? " " : "";

    switch (type_) {
        case JsonType::Null: return "null";
        case JsonType::Bool: return bool_val_ ? "true" : "false";
        case JsonType::Number: {
            constexpr auto ll_min = static_cast<double>(std::numeric_limits<long long>::min());
            constexpr auto ll_max = static_cast<double>(std::numeric_limits<long long>::max());
            if (num_val_ >= ll_min && num_val_ <= ll_max &&
                num_val_ == static_cast<double>(static_cast<long long>(num_val_))) {
                return std::to_string(static_cast<long long>(num_val_));
            }
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%.10g", num_val_);
            return std::string(buf);
        }
        case JsonType::String: return escape(str_val_);
        case JsonType::Array: {
            if (arr_val_.empty()) return "[]";
            std::string out = "[" + nl;
            for (size_t i = 0; i < arr_val_.size(); ++i) {
                out += ind_next + arr_val_[i].dump(indent, depth + 1);
                if (i + 1 < arr_val_.size()) out += ",";
                out += nl;
            }
            return out + ind + "]";
        }
        case JsonType::Object: {
            if (obj_val_.empty()) return "{}";
            std::string out = "{" + nl;
            size_t i = 0;
            for (const auto& [k, v] : obj_val_) {
                out += ind_next + escape(k) + ":" + sp + v.dump(indent, depth + 1);
                if (++i < obj_val_.size()) out += ",";
                out += nl;
            }
            return out + ind + "}";
        }
    }
    return "null";
}

} // namespace ai
