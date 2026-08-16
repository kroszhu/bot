# Minimal QQ Bot

A minimal C++17 QQ bot that echoes direct text messages and group messages that
mention the bot. It follows the official QQ Bot WebSocket and OpenAPI flow.

## Build

Requirements: CMake 3.16+, a C++17 compiler, OpenSSL development files, and
internet access during the first CMake configure.

```bash
./build.sh
ctest --test-dir build --output-on-failure
```

## Run

Create a bot in the QQ Open Platform and enable the `GROUP_AND_C2C_EVENT`
intent. Keep credentials outside source control:

```bash
export QQBOT_APP_ID='your-app-id'
export QQBOT_CLIENT_SECRET='your-client-secret'
./run.sh
```

The process connects automatically and echoes supported incoming text. Stop it
with `Ctrl+C`.

## Format check

```bash
cmake --build build --target format-check
```
