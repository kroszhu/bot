#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace qqbot {

inline bool Startwith(std::string_view source, std::string_view target) {
  const std::size_t start = source.find_first_not_of(' ');
  return start != std::string_view::npos && source.find(target, start) == start;
}

inline std::string TruncateUtf8(std::string_view text, std::size_t max_bytes) {
  if (text.size() <= max_bytes) {
    return std::string(text);
  }

  std::size_t end = max_bytes;
  while (end > 0 && (static_cast<unsigned char>(text[end]) & 0xc0) == 0x80) {
    --end;
  }
  return std::string(text.substr(0, end));
}

}  // namespace qqbot
