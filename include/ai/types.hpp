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

struct UsageInfo {
    int prompt_tokens{0};
    int completion_tokens{0};
    int total_tokens{0};
    int cached_tokens{0};
    bool has_usage{false};
};

struct QuotaInfo {
    bool supported{false};
    bool success{false};
    std::string provider;
    std::string currency;
    std::string total_balance;
    std::string granted_balance;
    std::string topped_up_balance;
    std::string total_usage;
    std::string limit;
    std::string status;
    std::string info_message;
    std::string console_url;
    std::string error_message;
};

struct RequestOptions {
    std::string model;
    std::string system_prompt;
    double temperature{0.7};
    int max_tokens{4096};
    bool stream{true};
    std::string base_url;
    bool show_usage{false};
};

struct HttpResponse {
    long status_code{0};
    std::string body;
    std::string error;
    bool success{false};
};

using StreamCallback = std::function<void(const std::string& token)>;
using UsageCallback = std::function<void(const UsageInfo& usage)>;

} // namespace ai
