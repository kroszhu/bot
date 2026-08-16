#include "qqbot/config.h"

#include <cassert>
#include <string>

int main(int argc, char* argv[]) {
  assert(argc == 2);

  const qqbot::Config config(argv[1]);
  assert(config.Bot().app_id == "test-app-id");
  assert(config.Bot().client_secret == "test-client-secret");
  assert(config.Plugins().order.size() == 4);
  assert(config.Plugins().order[0] == "read");
  assert(config.Plugins().order[1] == "write");
  assert(config.Plugins().order[2] == "image");
  assert(config.Plugins().order[3] == "echo");
  return 0;
}
