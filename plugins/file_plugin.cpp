#include <fstream>
#include <iterator>
#include <string>

#include "qqbot/plugin.h"
#include "qqbot/string_utils.h"

namespace qqbot {
namespace {

constexpr char kFilePath[] = "data.txt";
constexpr char kReadCommand[] = "read";
constexpr char kWriteCommand[] = "write";

class ReadPlugin final : public Plugin {
 public:
  bool CanHandle(const Message& message) const override {
    return Startwith(message.content, kReadCommand);
  }

  void OnMessage(Client& client, const Message& message) override {
    std::ifstream file(kFilePath, std::ios::binary);
    if (!file) {
      client.Reply(message, "文件不存在");
      return;
    }

    const std::string text(std::istreambuf_iterator<char>(file), {});
    if (text.empty()) {
      client.Reply(message, "文件为空");
    } else {
      client.Reply(message, text);
    }
  }
};

class WritePlugin final : public Plugin {
 public:
  bool CanHandle(const Message& message) const override {
    return Startwith(message.content, kWriteCommand);
  }

  void OnMessage(Client& client, const Message& message) override {
    std::size_t text_position =
        message.content.find(kWriteCommand) + sizeof(kWriteCommand) - 1;
    while (text_position < message.content.size() &&
           message.content[text_position] == ' ') {
      ++text_position;
    }
    const std::string text = message.content.substr(text_position);
    if (text.empty()) {
      client.Reply(message, "用法：write 文本");
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
PluginRegister read_plugin_register(kReadCommand, &read_plugin);
WritePlugin write_plugin;
PluginRegister write_plugin_register(kWriteCommand, &write_plugin);

}  // namespace
}  // namespace qqbot
