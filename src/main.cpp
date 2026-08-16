#include <cstdlib>
#include <exception>
#include <iostream>

#include "qqbot/client.h"
#include "qqbot/plugin.h"

int main() {
  const char* app_id = std::getenv("QQBOT_APP_ID");
  const char* client_secret = std::getenv("QQBOT_CLIENT_SECRET");
  if (app_id == nullptr || client_secret == nullptr) {
    std::cerr << "QQBOT_APP_ID and QQBOT_CLIENT_SECRET are required.\n";
    return 1;
  }

  try {
    qqbot::Client client(app_id, client_secret);
    client.OnMessage([&client](const qqbot::Message& message) {
      for (const auto& [name, plugin] : qqbot::Plugins()) {
        if (plugin->CanHandle(message)) {
          plugin->OnMessage(client, message);
          return;
        }
      }
      // 兜底回复：
      client.Reply(message, "收到，但是暂时处理不了，等待升级");
    });
    client.Run();
  } catch (const std::exception& error) {
    std::cerr << "Fatal error: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
