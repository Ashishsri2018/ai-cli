#include "test_runner.hpp"
#include "ai/crypto.hpp"
#include "ai/config.hpp"
#include "ai/utils.hpp"

using namespace ai;

AI_TEST(CryptoSha256) {
    ASSERT_STREQ(crypto::sha256_hex("").c_str(), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    ASSERT_STREQ(crypto::sha256_hex("hello").c_str(), "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

AI_TEST(CryptoBase64Roundtrip) {
    std::string text = "Secret API Key 12345!@#$%^&*()";
    std::vector<uint8_t> bytes(text.begin(), text.end());
    std::string b64 = crypto::base64_encode(bytes);
    auto decoded = crypto::base64_decode(b64);
    std::string recovered(decoded.begin(), decoded.end());
    ASSERT_STREQ(recovered.c_str(), text.c_str());
}

AI_TEST(CryptoEncryptDecryptRoundtrip) {
    std::string key = "AIzaSyDummyGeminiKey123456789";
    std::string encrypted = crypto::encrypt_key(key);
    ASSERT_TRUE(crypto::is_encrypted(encrypted));
    ASSERT_TRUE(encrypted != key);

    std::string decrypted = crypto::decrypt_key(encrypted);
    ASSERT_STREQ(decrypted.c_str(), key.c_str());

    // Test raw string returns itself
    ASSERT_STREQ(crypto::decrypt_key(key).c_str(), key.c_str());
}

AI_TEST(ConfigEncryptedStorage) {
    std::string path = "/tmp/ai_test_enc_cfg.json";
    ConfigManager cm(path);
    std::string secret = "sk-proj-secret-key-123";
    cm.set_api_key("openai", secret);
    ASSERT_TRUE(cm.save());

    // Check on disk JSON to ensure key is stored encrypted
    std::string raw_json;
    ASSERT_TRUE(utils::read_file(path, raw_json));
    ASSERT_TRUE(raw_json.find("sk-proj-secret-key-123") == std::string::npos);
    ASSERT_TRUE(raw_json.find("enc:v1:") != std::string::npos);

    // Read back via ConfigManager to verify transparent decryption
    ConfigManager cm2(path);
    auto key_opt = cm2.get_api_key("openai");
    ASSERT_TRUE(key_opt.has_value());
    ASSERT_STREQ(key_opt->c_str(), secret.c_str());
}
