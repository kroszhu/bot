#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXSocketTLSOptions.h>

#include <iostream>
#include <string>

#include "llm_protocol.h"
#include "qqbot/config.h"
#include "qqbot/plugin.h"

namespace qqbot {
namespace {

constexpr char kPluginName[] = "llm";
constexpr char kChatCompletionsPath[] = "/v1/chat/completions";
constexpr char kCaBundle[] = "/etc/ssl/certs/ca-certificates.crt";

class LlmPlugin final : public Plugin {
 public:
  void Initialize(const Config& config) override {
    base_url_ = config.Llm().base_url;
    while (!base_url_.empty() && base_url_.back() == '/') {
      base_url_.pop_back();
    }
    timeout_ = config.Llm().timeout;
  }

  bool CanHandle(const Message&) const override { return true; }

  void OnMessage(Client& client, const Message& message) override {
    ix::HttpClient http_client;
    ix::SocketTLSOptions tls_options;
    tls_options.caFile = kCaBundle;
    http_client.setTLSOptions(tls_options);

    auto args = http_client.createRequest();
    args->connectTimeout = timeout_;
    args->transferTimeout = timeout_;
    args->extraHeaders["Content-Type"] = "application/json";
    const auto response = http_client.post(
        base_url_ + kChatCompletionsPath,
        internal::MakeLlmRequest(message.content).dump(), args);
    const int status = response ? response->statusCode : 0;
    const auto answer = response
                            ? internal::ParseLlmResponse(status, response->body)
                            : std::nullopt;
    if (answer) {
      client.Reply(message, *answer);
      return;
    }

    std::cerr << "LLM request failed, HTTP status: " << status << '\n';
    client.Reply(message, message.content);
  }

 private:
  std::string base_url_;
  int timeout_ = 30;
};

LlmPlugin llm_plugin;
PluginRegister llm_plugin_register(kPluginName, &llm_plugin);

}  // namespace
}  // namespace qqbot
