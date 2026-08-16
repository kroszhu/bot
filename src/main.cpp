#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "qqbot/client.h"
#include "qqbot/config.h"
#include "qqbot/plugin.h"

int main() {
  try {
    const qqbot::Config config("config.toml");
    std::vector<std::pair<std::string, qqbot::Plugin*>> plugins;
    for (const std::string& name : config.Plugins().order) {
      const auto plugin = qqbot::Plugins().find(name);
      if (plugin == qqbot::Plugins().end()) {
        throw std::runtime_error("unknown plugin: " + name);
      }
      plugins.emplace_back(name, plugin->second);
    }

    qqbot::Client client(config.Bot().app_id, config.Bot().client_secret);
    client.OnMessage([&client, plugins](const qqbot::Message& message) {
      for (auto& [name, plugin] : plugins) {
        if (plugin->CanHandle(message)) {
          std::cout << "Handling message plugin: " << name << "=>"
                    << message.content << std::endl;
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
