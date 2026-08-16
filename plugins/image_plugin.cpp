#include "qqbot/plugin.h"
#include "qqbot/string_utils.h"

namespace qqbot {
namespace {

constexpr char kImageCommand[] = "image";
constexpr char kImageUrl[] =
    "https://img2.baidu.com/it/u=3041022324,602489062&fm=253&fmt=auto&app="
    "138&f=PNG?w=281&h=499";

class ImagePlugin final : public Plugin {
 public:
  bool CanHandle(const Message& message) const override {
    return Startwith(message.content, kImageCommand);
  }

  void OnMessage(Client& client, const Message& message) override {
    client.ReplyImage(message, kImageUrl);
  }
};

ImagePlugin image_plugin;
PluginRegister image_plugin_register(kImageCommand, &image_plugin);

}  // namespace
}  // namespace qqbot
