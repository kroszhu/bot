#include "qqbot/plugin.h"

namespace qqbot {
namespace {

class HelpPlugin final : public Plugin {
 public:
  bool CanHandle(const Message& message) const override {
    return message.content == "help";
  }

  void OnMessage(Client& client, const Message& message) override {
    client.Reply(message,
                 "支持的用法：\n"
                 "help：查看帮助\n"
                 "read：读取 data.txt\n"
                 "write 文本：覆盖写入，最多1024字节\n"
                 "图片：回复一张图片\n"
                 "其他文本：原文回复");
  }
};

HelpPlugin help_plugin;
PluginRegister help_plugin_register("help", &help_plugin);

}  // namespace
}  // namespace qqbot
