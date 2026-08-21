#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <optional>

namespace ai {

enum class Role {
    System,
    User,
    Assistant
};

inline std::string role_to_string(Role role) {
    switch (role) {
        case Role::System: return "system";
        case Role::User: return "user";
        case Role::Assistant: return "assistant";
    }
    return "user";
}

inline std::ostream& operator<<(std::ostream& os, const Role& role) {
    os << role_to_string(role);
    return os;
}

inline Role string_to_role(const std::string& str) {
    if (str == "system") return Role::System;
    if (str == "assistant" || str == "model") return Role::Assistant;
    return Role::User;
}

struct ChatMessage {
    Role role;
    std::string content;
};

struct RequestOptions {
    std::string model;
    std::string system_prompt;
    double temperature{0.7};
    int max_tokens{4096};
    bool stream{true};
    std::string base_url;
};

struct HttpResponse {
    long status_code{0};
    std::string body;
    std::string error;
    bool success{false};
};

using StreamCallback = std::function<void(const std::string& token)>;

} // namespace ai
