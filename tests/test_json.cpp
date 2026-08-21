#include "test_runner.hpp"
#include "ai/json.hpp"

using namespace ai;

AI_TEST(JsonBasicPrimitives) {
    Json null_val;
    ASSERT_TRUE(null_val.is_null());
    ASSERT_STREQ(null_val.dump().c_str(), "null");

    Json bool_val(true);
    ASSERT_TRUE(bool_val.is_bool());
    ASSERT_TRUE(bool_val.as_bool());
    ASSERT_STREQ(bool_val.dump().c_str(), "true");

    Json num_val(42);
    ASSERT_TRUE(num_val.is_number());
    ASSERT_EQ(num_val.as_int(), 42);
    ASSERT_STREQ(num_val.dump().c_str(), "42");

    Json str_val("hello world");
    ASSERT_TRUE(str_val.is_string());
    ASSERT_STREQ(str_val.as_string().c_str(), "hello world");
    ASSERT_STREQ(str_val.dump().c_str(), "\"hello world\"");
}

AI_TEST(JsonArrayAndObject) {
    Json arr = Json::array();
    arr.push_back(1);
    arr.push_back("two");
    arr.push_back(false);
    ASSERT_TRUE(arr.is_array());
    ASSERT_EQ(arr.size(), 3);
    ASSERT_EQ(arr[0].as_int(), 1);
    ASSERT_STREQ(arr[1].as_string().c_str(), "two");
    ASSERT_FALSE(arr[2].as_bool());

    Json obj = Json::object();
    obj["name"] = "ai";
    obj["version"] = 1;
    ASSERT_TRUE(obj.is_object());
    ASSERT_TRUE(obj.has("name"));
    ASSERT_STREQ(obj["name"].as_string().c_str(), "ai");
    ASSERT_EQ(obj["version"].as_int(), 1);
}

AI_TEST(JsonParsing) {
    std::string json_str = R"({"provider": "google", "model": "gemini-2.5-flash", "temperature": 0.7, "stream": true, "list": [1, 2, 3]})";
    std::string err;
    Json root = Json::parse(json_str, err);
    ASSERT_TRUE(err.empty());
    ASSERT_TRUE(root.is_object());
    ASSERT_STREQ(root["provider"].as_string().c_str(), "google");
    ASSERT_STREQ(root["model"].as_string().c_str(), "gemini-2.5-flash");
    ASSERT_TRUE(root["stream"].as_bool());
    ASSERT_EQ(root["list"].size(), 3);
    ASSERT_EQ(root["list"][0].as_int(), 1);
}

AI_TEST(JsonEscaping) {
    std::string raw = "Line 1\nLine 2\t\"quoted\"";
    Json j(raw);
    std::string dumped = j.dump();
    ASSERT_TRUE(dumped.find("\\n") != std::string::npos);
    ASSERT_TRUE(dumped.find("\\\"") != std::string::npos);

    std::string err;
    Json parsed = Json::parse(dumped, err);
    ASSERT_TRUE(err.empty());
    ASSERT_STREQ(parsed.as_string().c_str(), raw.c_str());
}

AI_TEST(JsonMalformedUnicode) {
    // Malformed unicode escape should set error, not crash
    std::string err;
    Json result = Json::parse(R"("\uZZZZ")", err);
    ASSERT_FALSE(err.empty());
}

AI_TEST(JsonMismatchedBrackets) {
    std::string err;
    Json r1 = Json::parse("[1, 2, 3}", err);
    ASSERT_FALSE(err.empty());

    std::string err2;
    Json r2 = Json::parse("{\"a\": 1]", err2);
    ASSERT_FALSE(err2.empty());
}

AI_TEST(JsonUnterminatedString) {
    std::string err;
    Json r = Json::parse(R"("hello)", err);
    ASSERT_FALSE(err.empty());
}

AI_TEST(JsonHugeNumber) {
    // Should serialize very large numbers without UB
    Json huge(1e100);
    std::string dumped = huge.dump();
    ASSERT_FALSE(dumped.empty());
    ASSERT_TRUE(dumped.find("1e") != std::string::npos || dumped.find("1E") != std::string::npos);

    // Roundtrip parse
    std::string err;
    Json parsed = Json::parse(dumped, err);
    ASSERT_TRUE(err.empty());
    ASSERT_TRUE(parsed.is_number());
}
