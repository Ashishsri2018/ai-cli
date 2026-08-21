#include "test_runner.hpp"
#include "ai/session.hpp"

using namespace ai;

AI_TEST(SessionManagement) {
    ChatSession session("You are a helpful assistant.");
    ASSERT_STREQ(session.get_system_prompt().c_str(), "You are a helpful assistant.");
    ASSERT_TRUE(session.empty());

    session.add_user_message("What is the capital of France?");
    session.add_assistant_message("Paris is the capital of France.");
    ASSERT_EQ(session.size(), 2);

    const auto& msgs = session.get_messages();
    ASSERT_EQ(msgs[0].role, Role::User);
    ASSERT_STREQ(msgs[0].content.c_str(), "What is the capital of France?");
    ASSERT_EQ(msgs[1].role, Role::Assistant);
    ASSERT_STREQ(msgs[1].content.c_str(), "Paris is the capital of France.");

    session.clear();
    ASSERT_TRUE(session.empty());
    ASSERT_STREQ(session.get_system_prompt().c_str(), "You are a helpful assistant.");
}
