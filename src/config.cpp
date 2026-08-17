#include "qqbot/config.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <toml++/toml.hpp>
#include <utility>

namespace qqbot {
namespace {

bool IsValidTextDbKey(const std::string& key) {
  return key.size() >= 20 && key.size() <= 60 &&
         std::all_of(key.begin(), key.end(), [](unsigned char character) {
           return std::isalnum(character) || character == '-' ||
                  character == '_';
         });
}

}  // namespace

Config::Config(const std::string& path) {
  const toml::table config = toml::parse_file(path);
  auto app_id = config["bot"]["app_id"].value<std::string>();
  auto client_secret = config["bot"]["client_secret"].value<std::string>();
  if (!app_id || app_id->empty() || !client_secret || client_secret->empty()) {
    throw std::runtime_error("bot credentials are required in " + path);
  }
  bot_.app_id = std::move(*app_id);
  bot_.client_secret = std::move(*client_secret);

  const toml::array* order = config["plugins"]["order"].as_array();
  if (order == nullptr) {
    throw std::runtime_error("plugins.order is required in " + path);
  }
  for (const auto& item : *order) {
    auto name = item.value<std::string>();
    if (!name) {
      throw std::runtime_error("plugin names must be strings");
    }
    plugins_.order.push_back(std::move(*name));
  }

  auto key = config["customer_service"]["key"].value<std::string>();
  if (!key || !IsValidTextDbKey(*key)) {
    throw std::runtime_error(
        "customer_service.key must contain 20-60 letters, digits, '-' or '_'");
  }
  customer_service_.key = std::move(*key);
}

const BotConfig& Config::Bot() const { return bot_; }

const PluginsConfig& Config::Plugins() const { return plugins_; }

const CustomerServiceConfig& Config::CustomerService() const {
  return customer_service_;
}

}  // namespace qqbot
