# Architecture

## System Overview

The simulator is a single-process C++17 application with four layers:

```
┌─────────────────────────────────────────────────┐
│                   frontEnd/                      │
│         Vanilla JS (MVC) ↔ WebSocket            │
├─────────────────────────────────────────────────┤
│                    net/                          │
│            socket (wsServer bridge)              │
├─────────────────────────────────────────────────┤
│                  trading/                        │
│     Market → OrderBook → Matching Engine         │
│           Trader (Random, User)                  │
├─────────────────────────────────────────────────┤
│                   core/                          │
│   Chunked_OrderPool · FlatHeap · CandleStick     │
│                     Wallet                       │
├─────────────────────────────────────────────────┤
│                   util/                          │
│          Config · Logger · Tools                 │
└─────────────────────────────────────────────────┘
```

## Core Layer (`src/core/`)

### Chunked_OrderPool

The central memory arena for all orders. Uses a **Structure-of-Arrays (SoA)** layout for cache efficiency — each hot field (price, quantity, timestamp, side, owner) is stored in a contiguous vector rather than per-order structs.

Orders are allocated in `Chunk`s (default capacity 20,000). Each chunk owns six vectors (one per field). New chunks are appended when the current one fills up.

**Key design: Handle-based access.** `OrderHandle` is a lightweight value type (pointer to chunk + index + pre-computed comparison key) that references an order without being a pointer to it. This means handles remain valid when chunks grow, unlike raw pointers to elements.

The comparison key (`cmp_key`) is pre-computed at allocation time:
- **Buy orders:** `key = (price << 32) | ~timestamp_bits` — higher price wins, older orders win ties.
- **Ask orders:** `key = (~price << 32) | ~timestamp_bits` — inverted price so lower asks rank higher.

### FlatHeap

A binary max-heap implemented on a flat `std::vector<OrderHandle>`. Used by OrderBook to maintain sorted buy and sell queues. The heap compares handles by their pre-computed `cmp_key`, avoiding any pool lookups during heap operations (sift up/down).

### CandleStick

An OHLCV (Open/High/Low/Close/Volume) accumulator. Updated incrementally as trades arrive — the first trade initializes all fields, subsequent trades adjust high/low/close. When a time period ends, the candle is pushed to history and reset.

### Wallet

Tracks a trader's balance as separate `dollars` and `cents` integers (cents always 0-99). Provides immutable arithmetic (`add`/`sub` return new pairs) and mutating operations (`Deposit`/`Withdraw`).

## Trading Layer (`src/trading/`)

### Market

A facade over a `vector<OrderBook>` — one per stock. Handles initialization, seed order generation, and delegates price/volume/candle queries to individual orderbooks.

### OrderBook

Per-stock order management and matching. Maintains two `FlatHeap` instances (`buy_orders`, `sell_orders`) and the current price, trade volume, and candlestick state.

**Matching engine** (`match_orders`):
1. While both heaps are non-empty and `best_bid >= best_ask`:
   - Peek top buy and top sell orders.
   - `trade_qty = min(buy_qty, sell_qty)`.
   - **Price priority:** If the buy order is newer (higher timestamp), trade at `best_ask`; otherwise trade at `best_bid`. This gives price improvement to the resting order.
   - Reduce quantities, pop fully-filled orders.
   - Notify both traders via `order_filled()`.
   - Update candlestick, price, and volume.

### Trader Hierarchy

- **`Trader`** (base): Owns a `Wallet` and a `vector<Quantity>` of positions. Virtual `trade_on_market()` and `order_filled()`.
- **`RandomTrader`**: AI strategy — generates 1-10 random trades per tick. 70% aggressive (market-taking, drives price movement), 30% passive (market-making, provides liquidity).
- **`UserTrader`**: Drains the socket's trade queue each tick. Places user-submitted orders and sends status updates (queued → on_market → filled).

## Net Layer (`src/net/`)

### socket (singleton)

Bridges the C99 wsServer library to C++. Runs as a WebSocket server; all communication is JSON text frames.

**Inbound:** `onmessage` callback → `parse_msg` → dispatches to `parse_set`, `parse_get`, or `parse_user`.
**Outbound:** `post(msg)` broadcasts to all connected clients via `ws_sendframe_bcast`.

User trades are enqueued in `user_trade_queue` (producer-consumer pattern with `UserTrader` as consumer).

See [WEBSOCKET_API.md](WEBSOCKET_API.md) for the complete message protocol.

## Util Layer (`src/util/`)

### Config (singleton)

All simulation parameters in one place. Supports CLI parsing (`load_args`), JSON save/load, and runtime mutation via WebSocket `set` commands. Accessed by virtually every component.

### Logger (static)

Multi-sink spdlog logger: color console + rotating file (`logs/app-{host}-{port}.log`). Supports JSON log format via `LOG_JSON` env var.

### Tools

Bootstrap function `init_config_and_logger()` called from `main()`. Also provides JSON string helpers and range-check macros.

## Data Flow

```
UserTrader ←── user_trade_queue ←── socket (WebSocket)
    │
    ▼
Market.place_order()
    │
    ▼
OrderBook.place_order() → FlatHeap.push()
    │
    ▼
OrderBook.match_orders()
    │
    ├── Trader.order_filled() → Wallet update
    │
    └── CandleStick.update() → OHLCV history
         │
         ▼
    sim.send_update() → socket.post() → WebSocket broadcast
```

## Singleton Access

Three components use Meyer's singletons (`static T& instance()`):

| Singleton | Purpose | Lifetime |
|---|---|---|
| `config` | Global configuration | Process lifetime |
| `sim` | Simulation orchestrator | Process lifetime |
| `socket` | WebSocket server | Process lifetime |

`Logger` uses a static `shared_ptr<spdlog::logger>` instead.

## Memory Management

- **Order pool:** `unique_ptr<Chunk>` owns chunks. Handles are value types.
- **Traders:** Raw `new`/`delete` managed by `sim`. `UserTrader` is stack-allocated inside `sim`.
- **No RAII for traders** — `deinitialize_sim()` manually deletes all non-user traders.
