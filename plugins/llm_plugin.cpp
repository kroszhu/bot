#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXSocketTLSOptions.h>

#include <iostream>
#include <string>

#include "llm_protocol.h"
#include "qqbot/config.h"
#include "qqbot/plugin.h"
#include "qqbot/tls_utils.h"

namespace qqbot {
namespace {

constexpr char kPluginName[] = "llm";
constexpr char kChatCompletionsPath[] = "/v1/chat/completions";

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
    tls_options.caFile = CaBundlePath();
    http_client.setTLSOptions(tls_options);

    auto args = http_client.createRequest();
    args->connectTimeout = timeout_;
    args->transferTimeout = timeout_;
    args->extraHeaders["Content-Type"] = "application/json";
    const auto response = http_client.post(
        base_url_ + kChatCompletionsPath,
        internal::MakeLlmRequest(message.content).dump(), args);
    if (!response || response->statusCode < 200 ||
        response->statusCode >= 300) {
      std::cerr << "LLM request failed, HTTP status: "
                << (response ? response->statusCode : 0) << '\n';
      client.Reply(message, message.content);
      return;
    }

    const auto answer = internal::ParseLlmResponse(response->body);
    if (answer) {
      client.Reply(message, *answer);
      return;
    }

    std::cerr << "LLM response is invalid\n";
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
