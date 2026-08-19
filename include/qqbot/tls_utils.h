#ifndef QQBOT_TLS_UTILS_H_
#define QQBOT_TLS_UTILS_H_

#include <fstream>
#include <stdexcept>
#include <string>

namespace qqbot {

// mbedTLS 无法使用 "SYSTEM" 加载 macOS/Linux 系统根证书，必须提供 PEM 文件。
inline const std::string& CaBundlePath() {
  static const std::string path = [] {
    static constexpr const char* kCandidates[] = {
        "/etc/ssl/certs/ca-certificates.crt",        // Debian/Ubuntu/Alpine
        "/etc/ssl/certs/ca-bundle.crt",              // CentOS/RHEL
        "/etc/pki/tls/certs/ca-bundle.crt",          // Fedora/RHEL
        "/etc/ssl/cert.pem",                         // macOS / some BSD
        "/opt/homebrew/etc/ca-certificates/cert.pem",
        "/usr/local/etc/openssl@3/cert.pem",
        "/usr/local/etc/openssl/cert.pem",
    };
    for (const char* candidate : kCandidates) {
      if (std::ifstream(candidate).good()) {
        return std::string(candidate);
      }
    }
    throw std::runtime_error(
        "TLS CA bundle not found; install ca-certificates "
        "(e.g. /etc/ssl/cert.pem or /etc/ssl/certs/ca-certificates.crt)");
  }();
  return path;
}

}  // namespace qqbot

#endif  // QQBOT_TLS_UTILS_H_
