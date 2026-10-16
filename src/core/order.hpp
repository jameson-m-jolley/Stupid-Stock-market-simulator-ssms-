#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

using Quantity = std::uint32_t;
using Price = std::uint32_t;
using TraderId = std::uint32_t;
using Stock_N = uint16_t;
enum class Side { Buy, Ask };

class Trader; // forward declaration

/* Revised order pool
The old contigous simple memory arena did not account for dynamic resizing or
cache-hits. It used a AoS structure, an array of orders, but the new chunked
order-pool will be SoA.

Why chunked? The overhead & potential for pointers to dangle; once we need
dynamic resizing over a massive chunk of memory it undermines the gains from one
big block.

The new orderpool will return handles pointing to a chunk & the index of the
order data in the arrays. This means that the component can grab exactly what it
needs & have no extra access, further encapsulating the systems.

The chunked orderPool will encourage cache-hits & "no more than absolutely
necessary" access. The pool is involved everywhere orders are used,
you must either store it's reference or carry the reference with you into
functions.
*/

// The SoA chunk structure
struct Chunk {
  size_t capacity; // The capacity of this chunk
  size_t next = 0; // The next avaiable spot in this chunk

  // SoA fields
  std::vector<Stock_N> stocks;
  std::vector<Price> prices;
  std::vector<Quantity> quantities;
  std::vector<std::chrono::nanoseconds> time_stamps;
  std::vector<Side> sides;
  std::vector<Trader *> owners;
  // std::vector<bool> active;

  explicit Chunk(size_t cap)
      : capacity(cap), stocks(cap), prices(cap), quantities(cap),
        time_stamps(cap), sides(cap),
        owners(cap) {}; // instantiate all vector members silly ahh body c++
                        // moment all cap

  bool full() const { return next >= capacity; }

  // this works as the old pool[next++]
  size_t allocate() { return next++; }
};

struct OrderHandle {
  Chunk *chunk;
  size_t index;

  uint64_t cmp_key = 0; // price << 32 | timestamp (ns)

  void compute_key(const class Chunked_OrderPool &pool);
};

class Chunked_OrderPool {
private:
  std::vector<std::unique_ptr<Chunk>> chunks;
  size_t chunk_size;

public:
  explicit Chunked_OrderPool(size_t c_size = 20000);
  void reset();

  OrderHandle create(Stock_N stock, Price price, Quantity qty,
                     std::chrono::nanoseconds ts, Side side, Trader *);

  // Getters
  Stock_N stock(const OrderHandle &o) const { return o.chunk->stocks[o.index]; }
  Price price(const OrderHandle &o) const { return o.chunk->prices[o.index]; }
  Quantity quantity(const OrderHandle &o) const {
    return o.chunk->quantities[o.index];
  }
  std::chrono::nanoseconds timestamp(const OrderHandle &o) const {
    return o.chunk->time_stamps[o.index];
  }
  Side side(const OrderHandle &o) const { return o.chunk->sides[o.index]; }
  Trader *owner(const OrderHandle &o) const { return o.chunk->owners[o.index]; }
  // bool active(const OrderHandle &o) const { return o.chunk->active[o.index];
  // } //TTL in the future?
};