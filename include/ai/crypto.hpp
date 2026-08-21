#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace ai::crypto {

std::string sha256_hex(const std::string& input);
std::vector<uint8_t> sha256_bytes(const std::string& input);
std::string base64_encode(const std::vector<uint8_t>& data);
std::vector<uint8_t> base64_decode(const std::string& encoded);

std::vector<uint8_t> get_machine_key();
std::string encrypt_key(const std::string& plaintext);
std::string decrypt_key(const std::string& ciphertext);
bool is_encrypted(const std::string& text);

} // namespace ai::crypto
