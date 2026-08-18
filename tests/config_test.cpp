#include "qqbot/config.h"

#include <cassert>
#include <string>

int main(int argc, char* argv[]) {
  assert(argc == 2);

  const qqbot::Config config(argv[1]);
  assert(config.Bot().app_id == "test-app-id");
  assert(config.Bot().client_secret == "test-client-secret");
  assert(config.Plugins().order.size() == 7);
  assert(config.Plugins().order[0] == "write");
  assert(config.Plugins().order[1] == "help");
  assert(config.Plugins().order[2] == "read");
  assert(config.Plugins().order[3] == "image");
  assert(config.Plugins().order[4] == "customer_service");
  assert(config.Plugins().order[5] == "llm");
  assert(config.Plugins().order[6] == "echo");
  assert(config.CustomerService().key == "1234567890abcdefghij");
  assert(config.Llm().base_url == "http://192.168.1.20:8998/");
  assert(config.Llm().timeout == 30);
  return 0;
}
