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
intent. Copy the example configuration and fill in the credentials:

```bash
cp config.example.toml config.toml
./build/qqbot
```

The process connects automatically and echoes supported incoming text. Stop it
with `Ctrl+C`.

Use `read` to read `data.txt`, or `write text` to overwrite it. File content is
limited to 1024 bytes.

The `plugins.order` array controls plugin matching order. Each message is routed
to the first plugin whose `CanHandle` method returns true. New plugins inherit
`qqbot::Plugin` and register a static instance with `qqbot::PluginRegister`.

## Format check

```bash
cmake --build build --target format-check
```
