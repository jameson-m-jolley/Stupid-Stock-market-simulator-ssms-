#include "core/order.hpp"

// 63 ... 32 : 31 ... 0 : !Price : Time Stamp
// Fixed: some keys would compare wrong, so I reimplemented single comparator
// based keys
void OrderHandle::compute_key(const Chunked_OrderPool &pool) {
  Price price = pool.price(*this);
  Side side = pool.side(*this);
  uint64_t timestamp_ns = pool.timestamp(*this).count();
  uint32_t timestamp_bits = ~static_cast<uint32_t>(timestamp_ns & 0xFFFFFFFF);

  if (side == Side::Buy) {
    cmp_key = (static_cast<uint64_t>(price) << 32) | timestamp_bits;
  } else {
    Price inverted_price = ~price;
    cmp_key = (static_cast<uint64_t>(inverted_price) << 32) | timestamp_bits;
  }
}

Chunked_OrderPool::Chunked_OrderPool(size_t c_size) : chunk_size(c_size) {
  chunks.push_back(std::make_unique<Chunk>(chunk_size));
};

void Chunked_OrderPool::reset() {
  if (!chunks.empty()) {
    chunks[0]->next = 0;
    chunks.erase(chunks.begin() + 1, chunks.end());
  } else {
    chunks.push_back(std::make_unique<Chunk>(chunk_size));
  }
}

OrderHandle Chunked_OrderPool::create(Stock_N stock, Price price, Quantity qty,
                                      std::chrono::nanoseconds ts, Side side,
                                      Trader *owner) {
  Chunk *chunk =
      chunks.back()
          .get(); // grab the last chunk: back(), get it's pointer : get()

  // If the chunk is at capacity, push a new chunk and grab it (The dynamic
  // ability)
  if (chunk->full()) {
    chunks.push_back(std::make_unique<Chunk>(chunk_size));
    chunk = chunks.back().get();
  }

  size_t idx = chunk->allocate();
  chunk->stocks[idx] = stock;
  chunk->prices[idx] = price;
  chunk->quantities[idx] = qty;
  chunk->time_stamps[idx] = ts;
  chunk->owners[idx] = owner;
  chunk->sides[idx] = side;

  OrderHandle handle{chunk, idx};
  handle.compute_key(*this);

  return handle;
}