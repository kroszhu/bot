#include "qqbot/string_utils.h"

#include <cassert>

int main() {
  assert(qqbot::Startwith("help", "help"));
  assert(qqbot::Startwith("   help", "help"));
  assert(qqbot::Startwith(" help text", "help"));
  assert(!qqbot::Startwith("message help", "help"));
  assert(!qqbot::Startwith("   ", "help"));
  return 0;
}
