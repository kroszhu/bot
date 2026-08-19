#include "qqbot/plugin.h"
#include "qqbot/string_utils.h"

namespace qqbot {
namespace {

constexpr char kHelpCommand[] = "help";

class HelpPlugin final : public Plugin {
 public:
  bool CanHandle(const Message& message) const override {
    return Startwith(message.content, kHelpCommand);
  }

  void OnMessage(Client& client, const Message& message) override {
    client.Reply(message,
                 "支持的用法：\n"
                 "help：查看帮助\n"
                 "read：读取 data.txt\n"
                 "write 文本：覆盖写入\n"
                 "image：回复一张图片\n"
                 "客服问答：直接输入问题\n"
                 "faq-refresh：刷新客服问答\n"
                 "其他文本：原文回复");
  }
};

HelpPlugin help_plugin;
PluginRegister help_plugin_register(kHelpCommand, &help_plugin);

}  // namespace
}  // namespace qqbot
