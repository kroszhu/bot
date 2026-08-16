#include "qqbot/plugin.h"

namespace qqbot {
namespace {

class EchoPlugin final : public Plugin {
 public:
  bool CanHandle(const Message&) const override { return true; }

  void OnMessage(Client& client, const Message& message) override {
    client.Reply(message, message.content);
  }
};

EchoPlugin echo_plugin;
PluginRegister echo_plugin_register("echo", &echo_plugin);

}  // namespace
}  // namespace qqbot
