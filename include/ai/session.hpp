#pragma once

#include <vector>
#include <string>
#include "ai/types.hpp"

namespace ai {

class ChatSession {
public:
    ChatSession() = default;
    explicit ChatSession(std::string system_prompt);

    void set_system_prompt(const std::string& prompt);
    const std::string& get_system_prompt() const;

    void add_user_message(const std::string& content);
    void add_assistant_message(const std::string& content);
    void add_message(Role role, const std::string& content);

    const std::vector<ChatMessage>& get_messages() const;
    void clear();
    size_t size() const;
    bool empty() const;

private:
    std::string system_prompt_;
    std::vector<ChatMessage> messages_;
};

} // namespace ai
