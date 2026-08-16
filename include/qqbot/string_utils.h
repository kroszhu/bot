#ifndef QQBOT_STRING_UTILS_H_
#define QQBOT_STRING_UTILS_H_

#include <cstddef>
#include <string_view>

namespace qqbot {

inline bool Startwith(std::string_view source, std::string_view target) {
  const std::size_t start = source.find_first_not_of(' ');
  return start != std::string_view::npos && source.find(target, start) == start;
}

}  // namespace qqbot

#endif  // QQBOT_STRING_UTILS_H_
