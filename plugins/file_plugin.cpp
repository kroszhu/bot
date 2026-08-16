#include <cstddef>
#include <fstream>
#include <string>

#include "qqbot/plugin.h"

namespace qqbot {
namespace {

constexpr char kFilePath[] = "data.txt";
constexpr std::size_t kMaxTextSize = 1024;

class ReadPlugin final : public Plugin {
 public:
  bool CanHandle(const Message& message) const override {
    return message.content == "read";
  }

  void OnMessage(Client& client, const Message& message) override {
    std::ifstream file(kFilePath, std::ios::binary);
    if (!file) {
      client.Reply(message, "文件不存在");
      return;
    }

    std::string text(kMaxTextSize + 1, '\0');
    file.read(text.data(), static_cast<std::streamsize>(text.size()));
    text.resize(static_cast<std::size_t>(file.gcount()));
    if (text.size() > kMaxTextSize) {
      client.Reply(message, "文件内容超过1024字节");
    } else if (text.empty()) {
      client.Reply(message, "文件为空");
    } else {
      client.Reply(message, text);
    }
  }
};

class WritePlugin final : public Plugin {
 public:
  bool CanHandle(const Message& message) const override {
    return message.content == "write" ||
           message.content.rfind("write ", 0) == 0;
  }

  void OnMessage(Client& client, const Message& message) override {
    const std::string text =
        message.content.size() > 6 ? message.content.substr(6) : "";
    if (text.empty()) {
      client.Reply(message, "用法：write 文本");
      return;
    }
    if (text.size() > kMaxTextSize) {
      client.Reply(message, "写入内容不能超过1024字节");
      return;
    }

    std::ofstream file(kFilePath, std::ios::binary | std::ios::trunc);
    file.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!file) {
      client.Reply(message, "写入失败");
      return;
    }
    client.Reply(message, "写入成功");
  }
};

ReadPlugin read_plugin;
PluginRegister read_plugin_register("read", &read_plugin);
WritePlugin write_plugin;
PluginRegister write_plugin_register("write", &write_plugin);

}  // namespace
}  // namespace qqbot
