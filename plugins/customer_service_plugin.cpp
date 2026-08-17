#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXSocketTLSOptions.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "qqbot/config.h"
#include "qqbot/plugin.h"
#include "qqbot/string_utils.h"

namespace qqbot {
namespace {

constexpr char kPluginName[] = "customer_service";
constexpr char kRefreshCommand[] = "faq-refresh";
constexpr char kFaqFilePath[] = "faq.txt";
constexpr char kFaqBaseUrl[] = "https://textdb.online/";
constexpr char kCaBundle[] = "/etc/ssl/certs/ca-certificates.crt";
constexpr int kHttpTimeoutSeconds = 5;
constexpr auto kRefreshLimit = std::chrono::seconds(60);

using Faq = std::vector<std::pair<std::string, std::string>>;

Faq ParseFaq(std::istream& input) {
  Faq faq;
  std::string line;
  while (std::getline(input, line)) {
    const std::size_t separator = line.find('=');
    if (line.empty() || line.front() == '#' || separator == 0 ||
        separator == std::string::npos) {
      continue;
    }

    std::string question = line.substr(0, separator);
    std::string answer = line.substr(separator + 1);
    if (!answer.empty() && answer.back() == '\r') {
      answer.pop_back();
    }
    if (!answer.empty()) {
      faq.emplace_back(std::move(question), std::move(answer));
    }
  }
  return faq;
}

class CustomerServicePlugin final : public Plugin {
 public:
  CustomerServicePlugin() {
    std::ifstream file(kFaqFilePath);
    faq_ = ParseFaq(file);
  }

  void Initialize(const Config& config) override {
    faq_url_ = std::string(kFaqBaseUrl) + config.CustomerService().key;
    Refresh();
  }

  bool CanHandle(const Message& message) const override {
    return Startwith(message.content, kRefreshCommand) ||
           FindAnswer(message.content).has_value();
  }

  void OnMessage(Client& client, const Message& message) override {
    if (Startwith(message.content, kRefreshCommand)) {
      if (!BeginManualRefresh()) {
        client.Reply(message, "刷新过于频繁，请稍后再试");
      } else if (Refresh()) {
        client.Reply(message, "客服问答刷新成功");
      } else {
        client.Reply(message, "客服问答刷新失败，继续使用现有内容");
      }
      return;
    }

    const auto answer = FindAnswer(message.content);
    if (answer) {
      client.Reply(message, *answer);
    }
  }

 private:
  bool BeginManualRefresh() {
    const auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);
    if (last_manual_refresh_ && now - *last_manual_refresh_ < kRefreshLimit) {
      return false;
    }
    last_manual_refresh_ = now;
    return true;
  }

  bool Refresh() {
    ix::HttpClient client;
    ix::SocketTLSOptions tls_options;
    tls_options.caFile = kCaBundle;
    client.setTLSOptions(tls_options);

    auto args = client.createRequest();
    args->connectTimeout = kHttpTimeoutSeconds;
    args->transferTimeout = kHttpTimeoutSeconds;
    args->extraHeaders["Cache-Control"] = "no-cache";
    args->extraHeaders["User-Agent"] = "qqbot/0.1";
    const auto response = client.get(faq_url_, args);
    if (!response || response->statusCode != 200) {
      const int status = response ? response->statusCode : 0;
      std::cerr << "FAQ download failed, HTTP status: " << status << '\n';
      return false;
    }

    std::istringstream input(response->body);
    Faq new_faq = ParseFaq(input);
    if (new_faq.empty()) {
      std::cerr << "FAQ download returned no valid entries.\n";
      return false;
    }

    {
      std::lock_guard<std::mutex> lock(mutex_);
      faq_ = std::move(new_faq);
    }
    std::cout << "FAQ loaded from TextDB.\n";
    return true;
  }

  std::optional<std::string> FindAnswer(std::string_view input) const {
    const std::size_t start = input.find_first_not_of(' ');
    if (start == std::string_view::npos) {
      return std::nullopt;
    }
    input.remove_prefix(start);
    while (!input.empty() && input.back() == ' ') {
      input.remove_suffix(1);
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [question, answer] : faq_) {
      if (std::string_view(question).find(input) != std::string_view::npos) {
        return answer;
      }
    }
    return std::nullopt;
  }

  mutable std::mutex mutex_;
  Faq faq_;
  std::string faq_url_;
  std::optional<std::chrono::steady_clock::time_point> last_manual_refresh_;
};

CustomerServicePlugin customer_service_plugin;
PluginRegister customer_service_plugin_register(kPluginName,
                                                &customer_service_plugin);

}  // namespace
}  // namespace qqbot
