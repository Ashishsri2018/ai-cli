#include "ai/crypto.hpp"
#include "ai/utils.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <unistd.h>

namespace ai::crypto {

namespace {

const std::string kEncPrefix = "enc:v1:";
const std::string kSalt = "ai_cli_sec_salt_2026";

std::string get_host_id() {
    std::string id;
    if (utils::read_file("/etc/machine-id", id)) return utils::trim(id);
    if (utils::read_file("/var/lib/dbus/machine-id", id)) return utils::trim(id);
    char host[256];
    if (gethostname(host, sizeof(host)) == 0) id += host;
    id += ":" + utils::get_home_dir();
    return id.empty() ? "fallback_ai_device_id" : id;
}

std::vector<uint8_t> generate_keystream(const std::vector<uint8_t>& key, const std::vector<uint8_t>& nonce, size_t len) {
    std::vector<uint8_t> stream;
    uint32_t counter = 0;
    while (stream.size() < len) {
        std::string seed(key.begin(), key.end());
        seed.append(nonce.begin(), nonce.end());
        seed.append(reinterpret_cast<char*>(&counter), sizeof(counter));
        auto block = sha256_bytes(seed);
        stream.insert(stream.end(), block.begin(), block.end());
        counter++;
    }
    stream.resize(len);
    return stream;
}

} // namespace

std::string sha256_hex(const std::string& input) {
    auto b = sha256_bytes(input);
    std::ostringstream ss;
    for (uint8_t byte : b) ss << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    return ss.str();
}

std::vector<uint8_t> get_machine_key() {
    std::string seed = get_host_id() + ":" + kSalt;
    return sha256_bytes(seed);
}

bool is_encrypted(const std::string& text) {
    return utils::starts_with(text, kEncPrefix);
}

std::string encrypt_key(const std::string& plaintext) {
    if (plaintext.empty() || is_encrypted(plaintext)) return plaintext;
    auto key = get_machine_key();

    std::vector<uint8_t> nonce(12);
    std::random_device rd;
    for (size_t i = 0; i < nonce.size(); ++i) nonce[i] = static_cast<uint8_t>(rd() & 0xFF);

    auto keystream = generate_keystream(key, nonce, plaintext.size());
    std::vector<uint8_t> payload;
    payload.insert(payload.end(), nonce.begin(), nonce.end());
    for (size_t i = 0; i < plaintext.size(); ++i) {
        payload.push_back(static_cast<uint8_t>(plaintext[i] ^ keystream[i]));
    }
    return kEncPrefix + base64_encode(payload);
}

std::string decrypt_key(const std::string& ciphertext) {
    if (!is_encrypted(ciphertext)) return ciphertext;
    std::string b64 = ciphertext.substr(kEncPrefix.size());
    auto payload = base64_decode(b64);
    if (payload.size() <= 12) return "";

    std::vector<uint8_t> nonce(payload.begin(), payload.begin() + 12);
    std::vector<uint8_t> cipher(payload.begin() + 12, payload.end());
    auto key = get_machine_key();
    auto keystream = generate_keystream(key, nonce, cipher.size());

    std::string recovered;
    recovered.resize(cipher.size());
    for (size_t i = 0; i < cipher.size(); ++i) {
        recovered[i] = static_cast<char>(cipher[i] ^ keystream[i]);
    }
    return recovered;
}

} // namespace ai::crypto
