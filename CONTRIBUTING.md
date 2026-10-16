# Contributing

## Prerequisites

- [Zig 0.16+](https://ziglang.org/download/)
- A C++17 compiler (clang or gcc)
- Git
- clang-format (for code formatting)

## Building

```bash
git clone --recursive <repo-url>
cd team-project-stock-market-simulator
zig build
```

## Project Layout

```
src/
  core/        Data structures (OrderPool, FlatHeap, Wallet, CandleStick)
  trading/     Market, OrderBook, Trader classes
  net/         WebSocket server (socket)
  util/        Config, Logger, helper functions
  main.cpp     Entry point
  sim.hpp      Simulation orchestrator
frontEnd/      Browser frontend (vanilla JS MVC)
tests/         C++ test files using doctest
third_party/   Vendored dependencies (git submodules)
```

## Adding New Source Files

1. Place the file in the appropriate `src/` subdirectory.
2. Add the file path to `build.zig` in both the `exe_mod` source list and all `t_mod` test source lists.
3. Add the include path if the file is in a new subdirectory.

## Code Style

- C++17
- Compiled with `-Wall -Wextra -Werror` — warnings are errors
- clang-format with LLVM style: `zig build format`
- Test files go in `tests/` and use doctest

## Testing

```bash
zig build test
```

Each test file is compiled as a standalone executable with all project sources. Tests use doctest's `TEST_CASE` and `CHECK`/`REQUIRE` macros.

To add a new test:
1. Create `tests/MyTest.cpp`
2. Add the file to the `test_sources` array in `build.zig`
3. Include the doctest header with `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`

## Dependencies

All third-party libraries are git submodules in `third_party/`. To update:

```bash
git submodule update --remote
```

Do not add remote dependencies without team discussion.
