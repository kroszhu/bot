#include "protocol.h"

#include <cassert>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>

int main() {
  const auto identify = qqbot::internal::MakeIdentify("token");
  assert(identify.at("op") == 2);
  assert(identify.at("d").at("token") == "QQBot token");
  assert(identify.at("d").at("intents") == (1ULL << 25));

  assert(qqbot::internal::MakeHeartbeat(std::nullopt).at("d").is_null());
  assert(qqbot::internal::MakeHeartbeat(42).at("d") == 42);

  const auto resume = qqbot::internal::MakeResume("token", "session", 7);
  assert(resume.at("op") == 6);
  assert(resume.at("d").at("session_id") == "session");
  assert(resume.at("d").at("seq") == 7);

  assert(qqbot::internal::ParseExpiresIn({{"expires_in", 7200}}) == 7200);
  assert(qqbot::internal::ParseExpiresIn({{"expires_in", "7200"}}) == 7200);
  assert(!qqbot::internal::ParseExpiresIn({{"expires_in", "invalid"}}));

  const nlohmann::json direct_payload = {
      {"op", 0},
      {"t", "C2C_MESSAGE_CREATE"},
      {"d",
       {{"id", "message-id"},
        {"content", "hello"},
        {"message_type", 0},
        {"author", {{"user_openid", "user-id"}}}}}};
  const auto direct = qqbot::internal::ParseMessage(direct_payload);
  assert(direct.has_value());
  assert(direct->source == qqbot::MessageSource::kDirect);
  assert(direct->content == "hello");
  assert(qqbot::internal::ReplyPath(*direct) == "/v2/users/user-id/messages");
  assert(qqbot::internal::MakeReplyBody(*direct, "echo").at("msg_id") ==
         "message-id");

  const nlohmann::json group_payload = {{"op", 0},
                                        {"t", "GROUP_AT_MESSAGE_CREATE"},
                                        {"d",
                                         {{"id", "group-message-id"},
                                          {"content", "hello group"},
                                          {"message_type", 0},
                                          {"group_openid", "group-id"}}}};
  const auto group = qqbot::internal::ParseMessage(group_payload);
  assert(group.has_value());
  assert(group->source == qqbot::MessageSource::kGroup);
  assert(qqbot::internal::ReplyPath(*group) == "/v2/groups/group-id/messages");

  auto non_text = direct_payload;
  non_text["d"]["message_type"] = 7;
  assert(!qqbot::internal::ParseMessage(non_text).has_value());

  std::cout << "Protocol tests passed.\n";
  return 0;
}
