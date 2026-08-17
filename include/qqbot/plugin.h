#ifndef QQBOT_PLUGIN_H_
#define QQBOT_PLUGIN_H_

#include <map>
#include <string>

#include "qqbot/client.h"

namespace qqbot {

class Plugin {
 public:
  virtual ~Plugin() = default;
  virtual void Initialize() {}
  virtual bool CanHandle(const Message& message) const = 0;
  virtual void OnMessage(Client& client, const Message& message) = 0;
};

std::map<std::string, Plugin*>& Plugins();

class PluginRegister {
 public:
  PluginRegister(const std::string& name, Plugin* plugin);
};

}  // namespace qqbot

#endif  // QQBOT_PLUGIN_H_
