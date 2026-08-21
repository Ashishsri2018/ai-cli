#include "ai/http_client.hpp"
#include <curl/curl.h>
#include <mutex>

namespace ai {

namespace {
std::once_flag curl_init_flag_;
void curl_global_cleanup_handler() { curl_global_cleanup(); }
} // namespace

CurlHttpClient::CurlHttpClient() {
    std::call_once(curl_init_flag_, []() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        std::atexit(curl_global_cleanup_handler);
    });
}

CurlHttpClient::~CurlHttpClient() = default;

namespace {

size_t write_string_cb(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    static_cast<std::string*>(userp)->append(static_cast<char*>(contents), total_size);
    return total_size;
}

struct StreamContext {
    StreamChunkCallback callback;
    std::string full_response;
};

size_t write_stream_cb(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    auto* ctx = static_cast<StreamContext*>(userp);
    std::string chunk(static_cast<char*>(contents), total_size);
    ctx->full_response.append(chunk);
    if (ctx->callback) ctx->callback(chunk);
    return total_size;
}

struct CurlSlistGuard {
    curl_slist* list{nullptr};
    ~CurlSlistGuard() { if (list) curl_slist_free_all(list); }
};

void init_curl_common(CURL* curl, const std::string& url, curl_slist* slist, int timeout_sec) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(timeout_sec));
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
}

} // namespace

HttpResponse CurlHttpClient::get(const std::string& url, const std::vector<std::string>& headers, int timeout_sec) {
    HttpResponse resp;
    CURL* curl = curl_easy_init();
    if (!curl) { resp.error = "Failed to initialize CURL"; return resp; }

    CurlSlistGuard slist_guard;
    for (const auto& h : headers) slist_guard.list = curl_slist_append(slist_guard.list, h.c_str());

    init_curl_common(curl, url, slist_guard.list, timeout_sec);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_string_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp.body);

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp.status_code);
        resp.success = (resp.status_code >= 200 && resp.status_code < 300);
    } else {
        resp.error = curl_easy_strerror(res);
    }
    curl_easy_cleanup(curl);
    return resp;
}

HttpResponse CurlHttpClient::post(const std::string& url, const std::vector<std::string>& headers,
                                  const std::string& body, int timeout_sec) {
    HttpResponse resp;
    CURL* curl = curl_easy_init();
    if (!curl) { resp.error = "Failed to initialize CURL"; return resp; }

    CurlSlistGuard slist_guard;
    for (const auto& h : headers) slist_guard.list = curl_slist_append(slist_guard.list, h.c_str());

    init_curl_common(curl, url, slist_guard.list, timeout_sec);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_string_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp.body);

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp.status_code);
        resp.success = (resp.status_code >= 200 && resp.status_code < 300);
    } else {
        resp.error = curl_easy_strerror(res);
    }
    curl_easy_cleanup(curl);
    return resp;
}

HttpResponse CurlHttpClient::post_stream(const std::string& url, const std::vector<std::string>& headers,
                                         const std::string& body, StreamChunkCallback on_chunk, int timeout_sec) {
    HttpResponse resp;
    CURL* curl = curl_easy_init();
    if (!curl) { resp.error = "Failed to initialize CURL"; return resp; }

    CurlSlistGuard slist_guard;
    for (const auto& h : headers) slist_guard.list = curl_slist_append(slist_guard.list, h.c_str());

    init_curl_common(curl, url, slist_guard.list, timeout_sec);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));

    StreamContext ctx{std::move(on_chunk), ""};
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_stream_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp.status_code);
        resp.body = std::move(ctx.full_response);
        resp.success = (resp.status_code >= 200 && resp.status_code < 300);
    } else {
        resp.error = curl_easy_strerror(res);
    }
    curl_easy_cleanup(curl);
    return resp;
}

} // namespace ai
