#include "protocol.h"

#include <utility>

namespace qqbot::internal {

nlohmann::json MakeIdentify(const std::string& access_token) {
  return {{"op", 2},
          {"d",
           {{"token", "QQBot " + access_token},
            {"intents", kGroupAndC2cIntent},
            {"shard", {0, 1}}}}};
}

nlohmann::json MakeHeartbeat(std::optional<std::uint64_t> sequence) {
  return {{"op", 1}, {"d", sequence ? nlohmann::json(*sequence) : nullptr}};
}

nlohmann::json MakeResume(const std::string& access_token,
                          const std::string& session_id,
                          std::uint64_t sequence) {
  return {{"op", 6},
          {"d",
           {{"token", "QQBot " + access_token},
            {"session_id", session_id},
            {"seq", sequence}}}};
}

std::optional<Message> ParseMessage(const nlohmann::json& payload) {
  const std::string event = payload.value("t", "");
  if (event != "C2C_MESSAGE_CREATE" && event != "GROUP_AT_MESSAGE_CREATE") {
    return std::nullopt;
  }

  const auto data_it = payload.find("d");
  if (data_it == payload.end() || !data_it->is_object() ||
      data_it->value("message_type", -1) != 0) {
    return std::nullopt;
  }

  Message message;
  message.id = data_it->value("id", "");
  message.content = data_it->value("content", "");
  if (event == "C2C_MESSAGE_CREATE") {
    message.source = MessageSource::kDirect;
    const auto author = data_it->value("author", nlohmann::json::object());
    message.target_id = author.value("user_openid", author.value("id", ""));
  } else {
    message.source = MessageSource::kGroup;
    message.target_id = data_it->value("group_openid", "");
  }

  if (message.id.empty() || message.target_id.empty()) {
    return std::nullopt;
  }
  return message;
}

std::string ReplyPath(const Message& message) {
  if (message.source == MessageSource::kDirect) {
    return "/v2/users/" + message.target_id + "/messages";
  }
  return "/v2/groups/" + message.target_id + "/messages";
}

nlohmann::json MakeReplyBody(const Message& message, const std::string& text) {
  return {{"content", text},
          {"msg_type", 0},
          {"msg_id", message.id},
          {"msg_seq", 1}};
}

}  // namespace qqbot::internal
