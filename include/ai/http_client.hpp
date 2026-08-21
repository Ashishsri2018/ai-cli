#pragma once

#include <string>
#include <vector>
#include <functional>
#include "ai/types.hpp"

namespace ai {

using StreamChunkCallback = std::function<void(const std::string& raw_chunk)>;

class IHttpClient {
public:
    virtual ~IHttpClient() = default;
    virtual HttpResponse get(const std::string& url,
                             const std::vector<std::string>& headers,
                             int timeout_sec = 30) = 0;

    virtual HttpResponse post(const std::string& url,
                              const std::vector<std::string>& headers,
                              const std::string& body,
                              int timeout_sec = 60) = 0;

    virtual HttpResponse post_stream(const std::string& url,
                                     const std::vector<std::string>& headers,
                                     const std::string& body,
                                     StreamChunkCallback on_chunk,
                                     int timeout_sec = 120) = 0;
};

class CurlHttpClient : public IHttpClient {
public:
    CurlHttpClient();
    ~CurlHttpClient() override;

    HttpResponse get(const std::string& url,
                     const std::vector<std::string>& headers,
                     int timeout_sec = 30) override;

    HttpResponse post(const std::string& url,
                      const std::vector<std::string>& headers,
                      const std::string& body,
                      int timeout_sec = 60) override;

    HttpResponse post_stream(const std::string& url,
                             const std::vector<std::string>& headers,
                             const std::string& body,
                             StreamChunkCallback on_chunk,
                             int timeout_sec = 120) override;

private:
    static bool global_init_done_;
};

} // namespace ai
