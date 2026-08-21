#include "ai/session.hpp"

namespace ai {

ChatSession::ChatSession(std::string system_prompt)
    : system_prompt_(std::move(system_prompt)) {}

void ChatSession::set_system_prompt(const std::string& prompt) {
    system_prompt_ = prompt;
}

const std::string& ChatSession::get_system_prompt() const {
    return system_prompt_;
}

void ChatSession::add_user_message(const std::string& content) {
    messages_.push_back({Role::User, content});
}

void ChatSession::add_assistant_message(const std::string& content) {
    messages_.push_back({Role::Assistant, content});
}

void ChatSession::add_message(Role role, const std::string& content) {
    if (role == Role::System) {
        system_prompt_ = content;
    } else {
        messages_.push_back({role, content});
    }
}

const std::vector<ChatMessage>& ChatSession::get_messages() const {
    return messages_;
}

void ChatSession::clear() {
    messages_.clear();
}

size_t ChatSession::size() const {
    return messages_.size();
}

bool ChatSession::empty() const {
    return messages_.empty();
}

} // namespace ai
