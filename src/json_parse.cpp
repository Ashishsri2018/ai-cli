#include "ai/json.hpp"

namespace ai {

namespace {

struct JsonParser {
    const std::string& src;
    size_t pos{0};
    std::string err;

    void skip_ws() {
        while (pos < src.size() && (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r')) {
            pos++;
        }
    }

    char peek() { skip_ws(); return pos < src.size() ? src[pos] : '\0'; }
    char get() { skip_ws(); return pos < src.size() ? src[pos++] : '\0'; }

    void parse_unicode(std::string& s) {
        if (pos + 4 > src.size()) return;
        std::string hex = src.substr(pos, 4);
        pos += 4;
        unsigned int cp = std::stoul(hex, nullptr, 16);
        if (cp < 0x80) {
            s += static_cast<char>(cp);
        } else if (cp < 0x800) {
            s += static_cast<char>(0xC0 | (cp >> 6));
            s += static_cast<char>(0x80 | (cp & 0x3F));
        } else {
            s += static_cast<char>(0xE0 | (cp >> 12));
            s += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            s += static_cast<char>(0x80 | (cp & 0x3F));
        }
    }

    Json parse_str() {
        if (get() != '"') { err = "Expected '\"'"; return {}; }
        std::string s;
        while (pos < src.size()) {
            char c = src[pos++];
            if (c == '"') return Json(s);
            if (c == '\\' && pos < src.size()) {
                char esc = src[pos++];
                switch (esc) {
                    case '"': s += '"'; break;
                    case '\\': s += '\\'; break;
                    case '/': s += '/'; break;
                    case 'b': s += '\b'; break;
                    case 'f': s += '\f'; break;
                    case 'n': s += '\n'; break;
                    case 'r': s += '\r'; break;
                    case 't': s += '\t'; break;
                    case 'u': parse_unicode(s); break;
                    default: s += esc; break;
                }
            } else {
                s += c;
            }
        }
        err = "Unterminated string";
        return {};
    }

    Json parse_num() {
        skip_ws();
        size_t start = pos;
        if (pos < src.size() && (src[pos] == '-' || src[pos] == '+')) pos++;
        while (pos < src.size() && (isdigit(src[pos]) || src[pos] == '.' || src[pos] == 'e' || src[pos] == 'E' || src[pos] == '-' || src[pos] == '+')) {
            pos++;
        }
        try {
            return Json(std::stod(src.substr(start, pos - start)));
        } catch (...) {
            err = "Invalid number";
            return {};
        }
    }

    Json parse_arr() {
        get(); // '['
        Json arr = Json::array();
        if (peek() == ']') { get(); return arr; }
        while (pos < src.size()) {
            arr.push_back(parse_val());
            if (!err.empty()) return {};
            if (peek() == ']') { get(); return arr; }
            if (peek() == ',') { get(); continue; }
            err = "Expected ',' or ']'"; return {};
        }
        err = "Unterminated array"; return {};
    }

    Json parse_obj() {
        get(); // '{'
        Json obj = Json::object();
        if (peek() == '}') { get(); return obj; }
        while (pos < src.size()) {
            if (peek() != '"') { err = "Expected key"; return {}; }
            std::string key = parse_str().as_string();
            if (peek() != ':') { err = "Expected ':'"; return {}; }
            get();
            obj[key] = parse_val();
            if (!err.empty()) return {};
            if (peek() == '}') { get(); return obj; }
            if (peek() == ',') { get(); continue; }
            err = "Expected ',' or '}'"; return {};
        }
        err = "Unterminated object"; return {};
    }

    Json parse_val() {
        skip_ws();
        char c = peek();
        if (c == '{') return parse_obj();
        if (c == '[') return parse_arr();
        if (c == '"') return parse_str();
        if (c == 't' && src.substr(pos, 4) == "true") { pos += 4; return Json(true); }
        if (c == 'f' && src.substr(pos, 5) == "false") { pos += 5; return Json(false); }
        if (c == 'n' && src.substr(pos, 4) == "null") { pos += 4; return Json(nullptr); }
        if (c == '-' || isdigit(c)) return parse_num();
        err = std::string("Unexpected '") + c + "'";
        return {};
    }
};

} // namespace

Json Json::parse(const std::string& src, std::string& err) {
    JsonParser parser{src, 0, ""};
    Json result = parser.parse_val();
    err = parser.err;
    return result;
}

} // namespace ai
