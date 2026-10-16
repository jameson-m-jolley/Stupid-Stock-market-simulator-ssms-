# Stock Market Simulator

A real-time stock market simulator with AI-driven traders, candlestick charting, and a WebSocket-based frontend. The engine runs an order-matching loop with configurable traders, stocks, and market parameters, streaming live data to a browser-based dashboard.

## Prerequisites

- [Zig 0.16+](https://ziglang.org/download/) (build system)
- A C++17 compiler (clang or gcc, invoked by Zig)
- Git (for cloning with submodules)

## Quick Start

```bash
git clone --recursive https://github.com/NMSU-CS-CS371/team-project-stock-market-simulator.git
cd team-project-stock-market-simulator
zig build
```

If you already cloned without `--recursive`:

```bash
git submodule update --init --recursive
zig build
```

## Build Commands

| Command | Description |
|---|---|
| `zig build` | Build the `sim` executable and all test binaries |
| `zig build run -- [args]` | Build and run the simulator with optional CLI args |
| `zig build server` | Start in WebSocket mode (port 8080, 1s updates, indefinite) |
| `zig build test` | Build and run all test suites |
| `zig build clean` | Remove build artifacts |
| `zig build format` | Run `clang-format` on source files |

## Running

### CLI Mode (text output)

```bash
zig build run -- -m 1 -tr 10 -st 5 -sp 1000 -tw 500
```

### WebSocket Mode (browser frontend)

```bash
zig build server
```

This starts the server on `localhost:8080` and opens `frontEnd/index.html` in your default browser. The frontend connects to the WebSocket server and displays live candlestick charts.

If the page shows "Waiting for Server", click the hamburger button (top right) and press Connect.

## Command-Line Flags

| Flag | Description | Default |
|---|---|---|
| `-m` | Mode: `1` = CLI, `2` = WebSocket, `3` = Monte Carlo | `1` |
| `-d` | Enable debug output | off |
| `-u` | Update interval in ms (`0` = fastest) | `1000` |
| `-t` | Runtime in ms (`-1` = indefinite) | `-1` |
| `-p` | WebSocket listen port (mode 2 only) | `8080` |
| `-s` | Simulation size | `100` |
| `-f` | Path to JSON config file (creates if missing) | none |
| `-c` | Clamp invalid config values | off |
| `-seed` | Random seed (`0` = auto-generate) | `0` |
| `-tr` | Number of AI traders | `10` |
| `-st` | Number of stocks | `5` |
| `-sp` | Initial stock price in cents | `1000` ($10.00) |
| `-tw` | Initial wallet balance in dollars | `500` |
| `-so` | Seed orders per stock at startup | `5` |
| `-unsafe` | Ignore runtime checks | off |
| `-host` | WebSocket bind address | `localhost` |
| `-load` | Load config from file path | none |
| `-help` | Print help and exit | |
| `-mcr` | Monte Carlo runs (mode 3 only) | none |
| `-mcfsp` | Monte Carlo file save path (mode 3 only) | none |

## Dependencies

All dependencies are vendored as git submodules in `third_party/`:

| Library | Purpose |
|---|---|
| [wsServer](https://github.com/Theldus/wsServer) | C99 WebSocket server |
| [spdlog](https://github.com/gabime/spdlog) | Fast C++ logging |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON parsing and serialization |
| [doctest](https://github.com/doctest/doctest) | C++ testing framework (in `tests/doctest/`) |

## Project Structure

```
src/
  main.cpp              Entry point
  sim.hpp               Simulation orchestrator
  core/                 Data structures and primitives
    order.hpp/cpp         Order model and SoA memory pool
    wallet.hpp/cpp        Dollar/cents wallet
    candlestick.hpp       OHLCV candle
    FlatHeap.hpp          Binary heap on handles
  trading/              Market mechanics
    market.hpp            Market (collection of orderbooks)
    orderbook.hpp/cpp     Per-stock orderbook and matching engine
    trader.hpp/cpp        Trader hierarchy (Random, User)
  net/                  Networking
    socket.hpp/cpp        WebSocket server and message handling
  util/                 Utilities
    config.hpp/cpp        Configuration singleton
    logger.hpp/cpp        spdlog wrapper
    tools.hpp/cpp         Bootstrap and helper functions
frontEnd/               Browser frontend (vanilla JS, MVC)
  index.html            Main page
  model.js              Data model and WebSocket client
  view.js               UI rendering and chart
  controller.js         MVC controller
  index.css             Styles
third_party/            Vendored dependencies
tests/                  C++ test files (doctest)
old_docs/               Original documentation
```

See [ARCHITECTURE.md](ARCHITECTURE.md) for detailed design documentation and [WEBSOCKET_API.md](WEBSOCKET_API.md) for the WebSocket message protocol.
