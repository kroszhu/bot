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

class Config {
 public:
  explicit Config(const std::string& path);

  const BotConfig& Bot() const;
  const PluginsConfig& Plugins() const;
  const CustomerServiceConfig& CustomerService() const;

 private:
  BotConfig bot_;
  PluginsConfig plugins_;
  CustomerServiceConfig customer_service_;
};

}  // namespace qqbot

#endif  // QQBOT_CONFIG_H_
