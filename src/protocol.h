#ifndef QQBOT_SRC_PROTOCOL_H_
#define QQBOT_SRC_PROTOCOL_H_

#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "qqbot/client.h"

namespace qqbot::internal {

constexpr std::uint64_t kGroupAndC2cIntent = 1ULL << 25;

nlohmann::json MakeIdentify(const std::string& access_token);
nlohmann::json MakeHeartbeat(std::optional<std::uint64_t> sequence);
nlohmann::json MakeResume(const std::string& access_token,
                          const std::string& session_id,
                          std::uint64_t sequence);
std::optional<int> ParseExpiresIn(const nlohmann::json& response);
std::optional<Message> ParseMessage(const nlohmann::json& payload);
std::string ReplyPath(const Message& message);
nlohmann::json MakeReplyBody(const Message& message, const std::string& text);

}  // namespace qqbot::internal

#endif  // QQBOT_SRC_PROTOCOL_H_
