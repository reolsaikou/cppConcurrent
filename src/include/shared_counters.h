#include "common.h"
#include <array>
#include <atomic>
#include <cstdint>

template <typename T> struct ShardsPerThread {
  struct alignas(CACHE_LINE_SIZE) slot {
    T value;
    std::atomic<T> value_atomic;
  };

  std::array<slot, COUNTERS_NUM> slots_;

public:
  explicit ShardsPerThread(T initial);
};

static_assert(sizeof(ShardsPerThread<uint64_t>::slot) == CACHE_LINE_SIZE);

template <typename T>
static inline void init_per_thread(ShardsPerThread<T> &shards_counters,
                                   T initial) {
  for (int i = 0; i < COUNTERS_NUM; i++) {
    shards_counters.slots_[i].value = initial;
    shards_counters.slots_[i].value_atomic.store(initial);
  }
}

template <typename T>
static inline void smp_init(ShardsPerThread<T> &shards_counters, T initial) {
  int i;

  spin_lock_init(&__thread_id_map_lock);
  __thread_id_map[0] = pthread_self();
  for (int i = 1; i < COUNTERS_NUM; i++) {
    __thread_id_map[i] = __THREAD_ID_MAP_EMPTY;
  }
  init_per_thread(shards_counters, initial);
  if (pthread_key_create(&thread_id_key, NULL) != 0) {
    perror("pthread_key_create");
    exit(EXIT_FAILURE);
  }
}

static inline auto read_count(ShardsPerThread<uint64_t> &shards) -> uint64_t {
  uint64_t sum = 0;
  for (auto &slot : shards.slots_) {
    sum += READ_ONCE(slot.value);
  }

  return sum;
}

static inline auto read_count_atomic(ShardsPerThread<uint64_t> &shards)
    -> uint64_t {
  uint64_t sum = 0;
  for (auto &slot : shards.slots_) {
    sum += slot.value_atomic.load(std::memory_order_relaxed);
  }

  return sum;
}
