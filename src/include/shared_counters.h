#include "common.h"
#include <array>

pthread_key_t thread_id_key;

template <typename T> struct ShardsPerThread {
  struct alignas(CACHE_LINE_SIZE) slot {
    T value;
  };

  std::array<slot, COUNTERS_NUM> slots_;

public:
  explicit ShardsPerThread(T initial);
};

template <typename T>
static inline void init_per_thread(ShardsPerThread<T> shards_counters,
                                   T initial) {
  for (int i = 0; i < COUNTERS_NUM; i++) {
    shards_counters.slots_[i].value = initial;
  }
}

template <typename T>
static inline void smp_init(ShardsPerThread<T> shards_counters, T initial) {
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
