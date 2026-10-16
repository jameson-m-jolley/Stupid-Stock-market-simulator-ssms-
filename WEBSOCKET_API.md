# WebSocket API

The simulator exposes a WebSocket server (default `ws://localhost:8080`). All messages are JSON text frames.

## Connection

```
ws://<host>:<port>
```

Host defaults to `localhost`, port defaults to `8080`. Configurable via `-host` and `-p` CLI flags.

## Client → Server Messages

All messages require a `"command"` field. Three commands: `set`, `get`, `user`.

### `set` — Update Configuration

```json
{
  "command": "set",
  "target": "<field>",
  "value": "<string_value>"
}
```

| Target | Effect |
|---|---|
| `running` | Start/stop the simulation loop (`"1"` = start, `"0"` = stop) |
| `paused` | Pause/unpause (`"1"` = paused, `"0"` = running) |
| `should_terminate` | Shut down the server (`"1"` = terminate) |
| `update_time` | Set simulation tick interval in ms |
| `mode` | Set simulation mode |
| `size` | Set simulation size |
| `debug` | Toggle debug output |
| `seed` | Set RNG seed |
| `port` | Set WebSocket port |
| `running_forever` | Toggle infinite runtime |
| `clamp_config` | Toggle config clamping |
| `unsafe` | Toggle runtime checks |
| `load_config` | Load config from file (value = file path) |
| `runtime` | No-op |

**Response:**
```json
{
  "server": {
    "command": "set",
    "target": "<field>",
    "value": "<string_value>"
  }
}
```

Always echoes the request back, regardless of whether the set succeeded.

### `get` — Request Data

```json
{
  "command": "get",
  "target": "<field>",
  "value": <optional>
}
```

| Target | Response |
|---|---|
| `wallet` | `{"get":{"target":"wallet","value":<cents>}}` |
| `market` | `{"get":{"target":"market","value":"stocks: <n>"}}` |
| `config` | `{"get":{"target":"config","value":{<config JSON>}}}` |
| `save` | `{"get":{"target":"save","value":"successful"|"error"}}` |
| `candles` | See below |

#### `get candles`

**Request forms:**
```json
{"command":"get","target":"candles","value":{"stock":0,"amountOfCandles":100}}
{"command":"get","target":"candles","value":50}
{"command":"get","target":"candles"}
```

Defaults: `stock` = `0`, `amountOfCandles` = `100`.

**Response:**
```json
{
  "sim": {
    "candles": {
      "stock": 0,
      "candles": [
        {
          "open": 1000,
          "high": 1050,
          "low": 980,
          "close": 1020,
          "num_trades": 15,
          "close_time": 1234567890
        }
      ]
    }
  }
}
```

Note: This response wraps under `"sim"`, not `"get"` — this is an inconsistency in the protocol.

### `user` — Submit Trade

```json
{
  "command": "user",
  "trade": {
    "stock": 0,
    "price": 1000,
    "quantity": 10,
    "side": "buy"
  }
}
```

`side` accepts `"buy"` or `"Buy"` for buy orders. Any other value (including `"sell"`, `"ask"`) maps to ask/sell.

**Response (queued):**
```json
{"user":{"tradeStatus":"queued"}}
```

**Response (error):**
```json
{"user":{"tradeStatus":"error","error":"missing fields"}}
```

## Server → Client Messages

### Intercept Acknowledgement

Sent for **every** received message, before any command-specific response:

```json
{"socket":"message was intercepted"}
```

This means each client message always triggers two server responses.

### Simulation Start

Sent once when the simulation initializes:

```json
{
  "sim": {
    "start": {
      "num_stocks": 5,
      "init_wallet": 500,
      "time_passed": 0
    }
  }
}
```

### Simulation Update

Sent every simulation tick:

```json
{
  "sim": {
    "update": {
      "time_passed": 1234567890,
      "status": "okay",
      "stock_data": {
        "prices": [1000, 2000, 1500],
        "volumes": [42, 38, 55]
      }
    }
  }
}
```

`status` is `"okay"` during normal operation or `"paused"` when paused.

### Trade Status Updates

Asynchronous updates for user-submitted trades:

```json
{"user":{"tradeStatus":"on_market"}}
```
```json
{
  "user": {
    "tradeStatus": "filled",
    "trade": {
      "stock": 0,
      "quantity": 10,
      "side": "buy",
      "wallet": [49000, 10]
    }
  }
}
```

### Error Responses

```json
{"error":"missing command"}
{"error":"missing target"}
{"error":"missing trade"}
```

Note: Error messages use a flat `{"error":...}` envelope and are **not** handled by the frontend's message dispatcher.

## Protocol Notes

1. **Double response:** Every message gets an acknowledgement plus the actual response.
2. **Inconsistent envelopes:** Most responses use `{"get":{...}}` or `{"server":{...}}`, but candle responses use `{"sim":{...}}` and errors use `{"error":...}`.
3. **Hardcoded get responses:** `get wallet` always returns `1500`, `get market` always returns `"stocks: 10"`.
4. **Loose side parsing:** Only `"buy"`/`"Buy"` maps to Buy; everything else is Ask.
5. **`runtime` set is a no-op** but still echoes in the response.
