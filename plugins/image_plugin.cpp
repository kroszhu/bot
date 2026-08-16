#include <string>

#include "qqbot/plugin.h"

namespace qqbot {
namespace {

constexpr char kImageUrl[] =
    "https://img2.baidu.com/it/u=3041022324,602489062&fm=253&fmt=auto&app="
    "138&f=PNG?w=281&h=499";

class ImagePlugin final : public Plugin {
 public:
  bool CanHandle(const Message& message) const override {
    return message.content.find("图片") != std::string::npos;
  }

  void OnMessage(Client& client, const Message& message) override {
    client.ReplyImage(message, kImageUrl);
  }
};

ImagePlugin image_plugin;
PluginRegister image_plugin_register("image", &image_plugin);

}  // namespace
}  // namespace qqbot
