#ifndef QQBOT_LLM_PROTOCOL_H_
#define QQBOT_LLM_PROTOCOL_H_

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace qqbot {
namespace internal {

nlohmann::json MakeLlmRequest(const std::string& content);
std::optional<std::string> ParseLlmResponse(int status_code,
                                            const std::string& body);

}  // namespace internal
}  // namespace qqbot

#endif  // QQBOT_LLM_PROTOCOL_H_
