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

Use `help` to view the available commands, `read` to read `data.txt`, or
`write text` to overwrite it. File content is limited to 1024 bytes.

Customer service questions and answers are stored in `faq.txt`, one
`question=answer` pair per line. The first question containing the user's input
provides the reply. Set a private random TextDB key in `config.toml`, then upload
the file with:

```bash
curl --location --post301 -X POST https://textdb.online/update/ \
  --data-urlencode "key=your-random-key" \
  --data-urlencode "value@faq.txt"
```

The bot downloads the latest data from TextDB at startup. Send `faq-refresh` to
download it again; manual refreshes are limited to once per minute. If a
download fails, the existing cached data is retained. TextDB removes free
records that have not been accessed or updated for 30 days.

The `plugins.order` array controls plugin matching order. Each message is routed
to the first plugin whose `CanHandle` method returns true. New plugins inherit
`qqbot::Plugin` and register a static instance with `qqbot::PluginRegister`.

## Local LLM

The `llm` plugin sends messages that were not answered by customer service to a
separately running MiniMind OpenAI-compatible server. Start it in the model
environment (not in the bot process):

```bash
cd model/scripts
python serve_openai_api.py --load_from ../minimind-3 --device cuda
```

Configure the address reachable from the bot environment:

```toml
[llm]
base_url = "http://192.168.1.20:8998"
timeout = 30
```

Allow TCP port 8998 between the environments. The model server and bot remain
independent, so either process can fail or run out of memory without terminating
the other. Each request currently contains only the latest user message; chat
history and streaming are not supported yet. If the model request fails, the
plugin logs the failure and echoes the original message.

## Format check

```bash
cmake --build build --target format-check
```
