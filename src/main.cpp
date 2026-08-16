#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include "qqbot/client.h"

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
      client.Reply(message, message.content);
    });
    client.Run();
  } catch (const std::exception& error) {
    std::cerr << "Fatal error: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
