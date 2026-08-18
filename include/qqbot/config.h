#ifndef QQBOT_CONFIG_H_
#define QQBOT_CONFIG_H_

#include <string>
#include <vector>

namespace qqbot {

struct BotConfig {
  std::string app_id;
  std::string client_secret;
};

struct PluginsConfig {
  std::vector<std::string> order;
};

struct CustomerServiceConfig {
  std::string key;
};

struct LlmConfig {
  std::string base_url;
  int timeout;
};

class Config {
 public:
  explicit Config(const std::string& path);

  const BotConfig& Bot() const;
  const PluginsConfig& Plugins() const;
  const CustomerServiceConfig& CustomerService() const;
  const LlmConfig& Llm() const;

 private:
  BotConfig bot_;
  PluginsConfig plugins_;
  CustomerServiceConfig customer_service_;
  LlmConfig llm_;
};

}  // namespace qqbot

#endif  // QQBOT_CONFIG_H_
