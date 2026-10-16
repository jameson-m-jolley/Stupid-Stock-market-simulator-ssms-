#pragma once

#include "core/order.hpp"

/*
Before the current implementation the comparators held a reference to the order
pool It would have to consult the order pool through a pointer for both
time-stamps & price

The key based compare creates a value representing the price & time-stamp at
OrderHandle creation that can be used throughout its entire lifetime. This
greatly decreased cache misses.

*/
struct OrderComparator {
  bool operator()(const OrderHandle &A, const OrderHandle &B) const noexcept {
    return A.cmp_key < B.cmp_key; // Always same comparison
  }
};

/* Context-Specific Binary Heap Implementation

The top/root element is the best order (L1 order)
This is the only level that matters, the tree will maintain level N structure.

It maintains level structure by asserting that all new elements are such that
its parent is the better order & its children are worse or equal

It will perform this by consulting the comparator that enforces the
condition

Upon inserting a new element it is a leaf of the tree, sifting upwards until it
meets the condition. Popping an element swaps the root & last element before
sifting downwards until it meets the condition.

This tree is array-based instead of node-based, making it flat. This
benefits the cpu by being held in contigous memory to utilize the
SoA structure and maximize cache-hits

This runs in O(log n) time but the cost is lower due to it's
cache utilization.

*everything is noexcept because all we do is simple instructions
 this lets the vector utilize moves instead of copies where applicable
 this means less overhead as orders aggregate within orderbooks

Shoutout to dr. tuan le
*/

template <typename T, typename Comparator> class FlatHeap {
private:
  std::vector<OrderHandle> data;
  Comparator cmp;

  void sift_up(size_t idx) {
    while (idx > 0) {
      size_t parent = (idx - 1) / 2;
      if (!cmp(data[parent], data[idx]))
        break;
      std::swap(data[parent], data[idx]);
      idx = parent;
    }
  }

  void sift_down(size_t idx) {
    const size_t n = data.size();
    while (true) {
      size_t left = 2 * idx + 1;
      size_t right = 2 * idx + 2;

      size_t best = idx;

      if (left < n && cmp(data[best], data[left]))
        best = left;
      if (right < n && cmp(data[best], data[right]))
        best = right;

      if (best == idx)
        break;
      std::swap(data[idx], data[best]);
      idx = best;
    }
  }

public:
  explicit FlatHeap(Comparator cmp = Comparator{}) noexcept : cmp{cmp} {}

  bool empty() const noexcept { return data.empty(); }
  size_t size() const noexcept { return data.size(); }

  const OrderHandle &top() const noexcept { return data.front(); }

  void push(const OrderHandle &o) {
    data.push_back(o);
    sift_up(data.size() - 1);
  }

  void pop() {
    if (data.empty())
      return;
    std::swap(data.front(), data.back());
    data.pop_back();
    if (!data.empty())
      sift_down(0);
  }

  void clear() noexcept { data.clear(); }
};