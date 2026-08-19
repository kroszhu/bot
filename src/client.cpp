#include "qqbot/client.h"

#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXSocketTLSOptions.h>
#include <ixwebsocket/IXWebSocket.h>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include "protocol.h"
#include "qqbot/tls_utils.h"

namespace qqbot {
namespace {

constexpr char kApiBase[] = "https://api.bot.qq.com";
constexpr char kGatewayUrl[] = "wss://api.bot.qq.com/websocket/";
constexpr auto kRefreshMargin = std::chrono::seconds(60);

void ConfigureTls(ix::HttpClient* client) {
  ix::SocketTLSOptions options;
  options.caFile = CaBundlePath();
  client->setTLSOptions(options);
}

std::string CurrentLocalTime() {
  const std::time_t current_time = std::time(nullptr);
  std::tm local_time = {};
  if (localtime_r(&current_time, &local_time) == nullptr) {
    return "unknown time";
  }

  char buffer[20] = {};
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local_time);
  return buffer;
}

}  // namespace

class Client::Impl {
 public:
  Impl(std::string app_id, std::string client_secret)
      : app_id_(std::move(app_id)), client_secret_(std::move(client_secret)) {
    if (app_id_.empty() || client_secret_.empty()) {
      throw std::invalid_argument("AppID and ClientSecret must not be empty");
    }
    ix::initNetSystem();
  }

  ~Impl() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopping_ = true;
    }
    heartbeat_cv_.notify_all();
    websocket_.stop();
    if (heartbeat_thread_.joinable()) {
      heartbeat_thread_.join();
    }
    ix::uninitNetSystem();
  }

  void OnMessage(MessageHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    message_handler_ = std::move(handler);
  }

  void Run() {
    RefreshAccessToken();
    websocket_.setUrl(kGatewayUrl);
    ix::SocketTLSOptions tls_options;
    tls_options.caFile = CaBundlePath();
    websocket_.setTLSOptions(tls_options);
    websocket_.enableAutomaticReconnection();
    websocket_.setOnMessageCallback(
        [this](const ix::WebSocketMessagePtr& event) { OnWebSocket(event); });
    std::cout << "Connecting to QQ Gateway...\n";
    websocket_.start();

    std::unique_lock<std::mutex> lock(mutex_);
    run_cv_.wait(lock, [this] { return stopping_; });
  }

  void Reply(const Message& message, const std::string& text) {
    try {
      EnsureAccessToken();
      const std::string token = GetAccessToken();
      ix::HttpClient client;
      ConfigureTls(&client);
      auto args = client.createRequest();
      args->extraHeaders["Authorization"] = "QQBot " + token;
      args->extraHeaders["Content-Type"] = "application/json";
      const auto response =
          client.post(std::string(kApiBase) + internal::ReplyPath(message),
                      internal::MakeReplyBody(message, text).dump(), args);
      if (!response || response->statusCode < 200 ||
          response->statusCode >= 300) {
        const int status = response ? response->statusCode : 0;
        std::cerr << "Reply failed, HTTP status: " << status << '\n';
      }
    } catch (const std::exception& error) {
      std::cerr << "Reply failed: " << error.what() << '\n';
    }
  }

  void ReplyImage(const Message& message, const std::string& image_url) {
    try {
      EnsureAccessToken();
      const std::string token = GetAccessToken();

      ix::HttpClient client;
      ConfigureTls(&client);
      auto args = client.createRequest();
      args->extraHeaders["Authorization"] = "QQBot " + token;
      args->extraHeaders["Content-Type"] = "application/json";

      const auto upload_response =
          client.post(std::string(kApiBase) + internal::UploadPath(message),
                      internal::MakeImageUploadBody(image_url).dump(), args);
      if (!upload_response || upload_response->statusCode < 200 ||
          upload_response->statusCode >= 300) {
        const int status = upload_response ? upload_response->statusCode : 0;
        std::cerr << "Image upload failed, HTTP status: " << status << '\n';
        return;
      }

      const auto upload_result = nlohmann::json::parse(upload_response->body);
      const std::string file_info = upload_result.value("file_info", "");
      if (file_info.empty()) {
        throw std::runtime_error("image upload returned no file_info");
      }

      const auto reply_response = client.post(
          std::string(kApiBase) + internal::ReplyPath(message),
          internal::MakeImageReplyBody(message, file_info).dump(), args);
      if (!reply_response || reply_response->statusCode < 200 ||
          reply_response->statusCode >= 300) {
        const int status = reply_response ? reply_response->statusCode : 0;
        std::cerr << "Image reply failed, HTTP status: " << status << '\n';
      }
    } catch (const std::exception& error) {
      std::cerr << "Image reply failed: " << error.what() << '\n';
    }
  }

 private:
  void RefreshAccessToken() {
    ix::HttpClient client;
    ConfigureTls(&client);
    auto args = client.createRequest();
    args->extraHeaders["Content-Type"] = "application/json";
    const nlohmann::json body = {{"appId", app_id_},
                                 {"clientSecret", client_secret_}};
    const auto response = client.post(
        std::string(kApiBase) + "/app/getAppAccessToken", body.dump(), args);
    if (!response || response->statusCode != 200) {
      throw std::runtime_error("unable to obtain access token");
    }

    const auto result = nlohmann::json::parse(response->body);
    const std::string token = result.value("access_token", "");
    const auto expires_in = internal::ParseExpiresIn(result);
    if (token.empty() || !expires_in) {
      throw std::runtime_error("invalid access token response");
    }

    std::lock_guard<std::mutex> lock(token_mutex_);
    access_token_ = token;
    token_expires_at_ =
        std::chrono::steady_clock::now() + std::chrono::seconds(*expires_in);
  }

  void EnsureAccessToken() {
    bool refresh = false;
    {
      std::lock_guard<std::mutex> lock(token_mutex_);
      refresh = access_token_.empty() ||
                std::chrono::steady_clock::now() + kRefreshMargin >=
                    token_expires_at_;
    }
    if (refresh) {
      RefreshAccessToken();
    }
  }

  std::string GetAccessToken() {
    std::lock_guard<std::mutex> lock(token_mutex_);
    return access_token_;
  }

  void OnWebSocket(const ix::WebSocketMessagePtr& event) {
    if (event->type == ix::WebSocketMessageType::Open) {
      std::cout << "Gateway connected.\n";
      return;
    }
    if (event->type == ix::WebSocketMessageType::Error) {
      std::cerr << "Gateway error: " << event->errorInfo.reason << '\n';
      return;
    }
    if (event->type == ix::WebSocketMessageType::Close) {
      std::cerr << '[' << CurrentLocalTime()
                << "] Gateway disconnected, code: " << event->closeInfo.code
                << '\n';
      return;
    }
    if (event->type != ix::WebSocketMessageType::Message) {
      return;
    }

    try {
      HandlePayload(nlohmann::json::parse(event->str));
    } catch (const std::exception& error) {
      std::cerr << "Invalid Gateway payload: " << error.what() << '\n';
    }
  }

  void HandlePayload(const nlohmann::json& payload) {
    if (payload.contains("s") && !payload["s"].is_null()) {
      std::lock_guard<std::mutex> lock(mutex_);
      sequence_ = payload["s"].get<std::uint64_t>();
    }

    const int opcode = payload.value("op", -1);
    if (opcode == 10) {
      const auto interval = std::chrono::milliseconds(
          payload.at("d").at("heartbeat_interval").get<std::uint64_t>());
      StartHeartbeat(interval);
      Authenticate();
      return;
    }
    if (opcode == 7 || opcode == 9) {
      if (opcode == 9) {
        std::lock_guard<std::mutex> lock(mutex_);
        session_id_.clear();
        sequence_.reset();
      }
      return;
    }
    if (opcode != 0) {
      return;
    }

    const std::string event_name = payload.value("t", "");
    if (event_name == "READY") {
      std::lock_guard<std::mutex> lock(mutex_);
      session_id_ = payload.at("d").value("session_id", "");
      std::cout << "Bot is ready.\n";
      return;
    }

    const auto message = internal::ParseMessage(payload);
    if (!message) {
      return;
    }
    MessageHandler handler;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      handler = message_handler_;
    }
    if (handler) {
      handler(*message);
    }
  }

  void Authenticate() {
    EnsureAccessToken();
    const std::string token = GetAccessToken();
    std::lock_guard<std::mutex> lock(mutex_);
    if (!session_id_.empty() && sequence_) {
      websocket_.send(
          internal::MakeResume(token, session_id_, *sequence_).dump());
    } else {
      websocket_.send(internal::MakeIdentify(token).dump());
    }
  }

  void StartHeartbeat(std::chrono::milliseconds interval) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      heartbeat_interval_ = interval;
      ++heartbeat_generation_;
    }
    heartbeat_cv_.notify_all();
    if (!heartbeat_thread_.joinable()) {
      heartbeat_thread_ = std::thread([this] { HeartbeatLoop(); });
    }
  }

  void HeartbeatLoop() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!stopping_) {
      const auto generation = heartbeat_generation_;
      const auto interval = heartbeat_interval_;
      if (heartbeat_cv_.wait_for(lock, interval, [this, generation] {
            return stopping_ || heartbeat_generation_ != generation;
          })) {
        continue;
      }
      const auto sequence = sequence_;
      lock.unlock();
      websocket_.send(internal::MakeHeartbeat(sequence).dump());
      lock.lock();
    }
  }

  std::string app_id_;
  std::string client_secret_;
  std::string access_token_;
  std::chrono::steady_clock::time_point token_expires_at_;
  std::mutex token_mutex_;

  ix::WebSocket websocket_;
  MessageHandler message_handler_;
  std::optional<std::uint64_t> sequence_;
  std::string session_id_;
  std::chrono::milliseconds heartbeat_interval_{0};
  std::uint64_t heartbeat_generation_ = 0;
  std::thread heartbeat_thread_;
  std::mutex mutex_;
  std::condition_variable heartbeat_cv_;
  std::condition_variable run_cv_;
  bool stopping_ = false;
};

Client::Client(std::string app_id, std::string client_secret)
    : impl_(std::make_unique<Impl>(std::move(app_id),
                                   std::move(client_secret))) {}

Client::~Client() = default;

void Client::OnMessage(MessageHandler handler) {
  impl_->OnMessage(std::move(handler));
}

void Client::Reply(const Message& message, const std::string& text) {
  impl_->Reply(message, text);
}

void Client::ReplyImage(const Message& message, const std::string& image_url) {
  impl_->ReplyImage(message, image_url);
}

void Client::Run() { impl_->Run(); }

}  // namespace qqbot
