#include "shared_counters.h"

template <typename T> ShardsPerThread<T>::ShardsPerThread(T initial) {
  for (auto &slot : slots_) {
    slot.value = initial;
  }
}
