#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <toml++/toml.hpp>
#include <vector>

#include "qqbot/client.h"
#include "qqbot/plugin.h"

int main() {
  try {
    const toml::table config = toml::parse_file("config.toml");
    const auto app_id = config["bot"]["app_id"].value<std::string>();
    const auto client_secret =
        config["bot"]["client_secret"].value<std::string>();
    if (!app_id || app_id->empty() || !client_secret ||
        client_secret->empty()) {
      throw std::runtime_error("bot credentials are required in config.toml");
    }

    const toml::array* order = config["plugins"]["order"].as_array();
    if (order == nullptr) {
      throw std::runtime_error("plugins.order is required in config.toml");
    }

    std::vector<qqbot::Plugin*> plugins;
    for (const auto& item : *order) {
      const auto name = item.value<std::string>();
      if (!name) {
        throw std::runtime_error("plugin names must be strings");
      }
      const auto plugin = qqbot::Plugins().find(*name);
      if (plugin == qqbot::Plugins().end()) {
        throw std::runtime_error("unknown plugin: " + *name);
      }
      plugins.push_back(plugin->second);
    }

    qqbot::Client client(*app_id, *client_secret);
    client.OnMessage([&client, plugins](const qqbot::Message& message) {
      for (qqbot::Plugin* plugin : plugins) {
        if (plugin->CanHandle(message)) {
          plugin->OnMessage(client, message);
          return;
        }
      }
      client.Reply(message, "收到，但是暂时处理不了，等待升级");
    });
    client.Run();
  } catch (const std::exception& error) {
    std::cerr << "Fatal error: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
