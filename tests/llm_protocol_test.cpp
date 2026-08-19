#include "llm_protocol.h"

#include <cassert>
#include <string>

int main() {
  const auto request = qqbot::internal::MakeLlmRequest("你好");
  assert(request.at("model") == "minimind");
  assert(request.at("stream") == false);
  assert(request.at("messages").size() == 1);
  assert(request.at("messages").at(0).at("role") == "user");
  assert(request.at("messages").at(0).at("content") == "你好");

  const std::string success =
      R"({"choices":[{"message":{"role":"assistant","content":"您好"}}]})";
  const auto answer = qqbot::internal::ParseLlmResponse(success);
  assert(answer && *answer == "您好");
  assert(!qqbot::internal::ParseLlmResponse("not json"));
  assert(!qqbot::internal::ParseLlmResponse(R"({"choices":[]})"));
  assert(!qqbot::internal::ParseLlmResponse(
      R"({"choices":[{"message":{"content":""}}]})"));
  return 0;
}
