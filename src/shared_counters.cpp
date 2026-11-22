#include "shared_counters.h"

template <typename T> ShardsPerThread<T>::ShardsPerThread(T initial) {
  for (auto &slot : slots_) {
    slot.value = initial;
  }
}

template struct ShardsPerThread<uint64_t>;
