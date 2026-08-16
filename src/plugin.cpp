#include "qqbot/plugin.h"

namespace qqbot {

std::map<std::string, Plugin*>& Plugins() {
  static std::map<std::string, Plugin*> plugins;
  return plugins;
}

PluginRegister::PluginRegister(const std::string& name, Plugin* plugin) {
  Plugins()[name] = plugin;
}

}  // namespace qqbot
