#include "qqbot/plugin.h"

namespace qqbot {
namespace {

class DefaultPlugin final : public Plugin {
 public:
  bool CanHandle(const Message&) const override { return true; }

  void OnMessage(Client& client, const Message& message) override {
    client.Reply(message, message.content);
  }
};

DefaultPlugin default_plugin;
PluginRegister default_plugin_register("default", &default_plugin);

}  // namespace
}  // namespace qqbot
