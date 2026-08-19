#include "qqbot/string_utils.h"

#include <cassert>
#include <string>

int main() {
  assert(qqbot::Startwith("help", "help"));
  assert(qqbot::Startwith("   help", "help"));
  assert(qqbot::Startwith(" help text", "help"));
  assert(!qqbot::Startwith("message help", "help"));
  assert(!qqbot::Startwith("   ", "help"));

  assert(qqbot::TruncateUtf8("hello", 5) == "hello");
  assert(qqbot::TruncateUtf8("hello", 4) == "hell");
  assert(qqbot::TruncateUtf8("中文", 6) == "中文");
  assert(qqbot::TruncateUtf8("中文", 5) == "中");
  assert(qqbot::TruncateUtf8("中文", 2).empty());

  const std::string long_text = std::string(1023, 'a') + "中";
  const std::string truncated = qqbot::TruncateUtf8(long_text, 1024);
  assert(truncated.size() == 1023);
  assert(truncated == std::string(1023, 'a'));
  return 0;
}
