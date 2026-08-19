#pragma once

#include <functional>
#include <memory>
#include <string>

namespace qqbot {

enum class MessageSource { kDirect, kGroup };

struct Message {
  MessageSource source;
  std::string id;
  std::string target_id;
  std::string content;
};

class Client {
 public:
  using MessageHandler = std::function<void(const Message&)>;

  Client(std::string app_id, std::string client_secret);
  ~Client();

  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;

  void OnMessage(MessageHandler handler);
  void Reply(const Message& message, const std::string& text);
  void ReplyImage(const Message& message, const std::string& image_url);
  void Run();

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace qqbot
