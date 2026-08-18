#include "llm_protocol.h"

#include <exception>

namespace qqbot {
namespace internal {

nlohmann::json MakeLlmRequest(const std::string& content) {
  return {{"model", "minimind"},
          {"messages", {{{"role", "user"}, {"content", content}}}},
          {"stream", false}};
}

std::optional<std::string> ParseLlmResponse(int status_code,
                                            const std::string& body) {
  if (status_code < 200 || status_code >= 300) {
    return std::nullopt;
  }
  try {
    const auto response = nlohmann::json::parse(body);
    const auto& content =
        response.at("choices").at(0).at("message").at("content");
    if (!content.is_string()) {
      return std::nullopt;
    }
    std::string text = content.get<std::string>();
    if (text.empty()) {
      return std::nullopt;
    }
    return text;
  } catch (const std::exception&) {
    return std::nullopt;
  }
}

}  // namespace internal
}  // namespace qqbot
