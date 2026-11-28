#include "common.h"
#include "shared_counters.h"
#include <atomic>
#include <bits/types/struct_timeval.h>
#include <cstdio>
#include <fmt/core.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

constexpr uint64_t per_thread_update = 1e8;

constexpr uint64_t duration_time = 20 * 1000;

std::atomic<uint64_t> global_counter(0);

auto ClockMs() -> uint64_t {
  struct timeval tm;
  gettimeofday(&tm, nullptr);
  return static_cast<uint64_t>(tm.tv_sec * 1000) +
         static_cast<uint64_t>(tm.tv_usec / 1000);
}

struct CountMetrics {
  uint64_t start_time_;
  uint64_t duration_ms_;
  explicit CountMetrics(uint64_t duration_ms) : duration_ms_(duration_ms) {}

  void begin() { start_time_ = ClockMs(); }

  auto shouldFinish() -> bool {
    auto now = ClockMs();
    return now - start_time_ > duration_ms_;
  }
};

void *counter(void *args) {

  int id = smp_thread_id(thread_id_key);
  auto *thread_counter =
      &static_cast<ShardsPerThread<uint64_t> *>(args)->slots_.at(id);

  fprintf(stdout, "thread id: %d\n", smp_thread_id(thread_id_key));
  // void *value = pthread_getspecific(thread_id_key);
  // fprintf(stdout, "thread id key: %p\n", value);

  CountMetrics metrics(duration_time);

  metrics.begin();

  while (!metrics.shouldFinish()) {
    for (int i = 0; i < per_thread_update; i++) {
      WRITE_ONCE(thread_counter->value, thread_counter->value + 1);
    }
  }

  uint64_t result = READ_ONCE(thread_counter->value);

  fprintf(stdout, "count result: %" PRIu64 "\n", result);

  return nullptr;
}

void *counter_atomic(void *args) {

  int id = smp_thread_id(thread_id_key);
  auto *thread_counter =
      &static_cast<ShardsPerThread<uint64_t> *>(args)->slots_.at(id);

  fprintf(stdout, "thread id: %d\n", smp_thread_id(thread_id_key));
  // void *value = pthread_getspecific(thread_id_key);
  // fprintf(stdout, "thread id key: %p\n", value);

  CountMetrics metrics(duration_time);

  metrics.begin();

  while (!metrics.shouldFinish()) {
    for (int i = 0; i < per_thread_update; i++) {
      // WRITE_ONCE(thread_counter->value, thread_counter->value + 1);
      thread_counter->value_atomic.fetch_add(1, std::memory_order_relaxed);
    }
  }

  // uint64_t result = READ_ONCE(thread_counter->value);
  uint64_t result = thread_counter->value_atomic.load();

  fprintf(stdout, "count result: %" PRIu64 "\n", result);

  return nullptr;
}

void *counter_atomic_global(void *args) {

  int id = smp_thread_id(thread_id_key);
  auto *thread_counter =
      &static_cast<ShardsPerThread<uint64_t> *>(args)->slots_.at(id);

  fprintf(stdout, "thread id: %d\n", smp_thread_id(thread_id_key));
  // void *value = pthread_getspecific(thread_id_key);
  // fprintf(stdout, "thread id key: %p\n", value);

  CountMetrics metrics(duration_time);

  metrics.begin();

  while (!metrics.shouldFinish()) {
    for (int i = 0; i < per_thread_update; i++) {
      // WRITE_ONCE(thread_counter->value, thread_counter->value + 1);
      // thread_counter->value_atomic.fetch_add(1, std::memory_order_relaxed);
      global_counter.fetch_add(1);
    }
  }

  // uint64_t result = READ_ONCE(thread_counter->value);
  // uint64_t result = thread_counter->value_atomic.load();

  // fprintf(stdout, "count result: %llu\n", result);

  return nullptr;
}

int main() {
  ShardsPerThread<uint64_t> per_thread_counters(0);

  smp_init(per_thread_counters, 0ul);

  constexpr int N = COUNTERS_NUM;

  for (int i = 0; i < COUNTERS_NUM; i++) {
    pthread_create(&__thread_id_map[i], nullptr, counter, &per_thread_counters);
  }

  CountMetrics metrics(duration_time);
  int i = 0;
  metrics.begin();
  while (!metrics.shouldFinish()) {
    sleep(1);
    i++;
    uint64_t observe = read_count(per_thread_counters);
    // uint64_t observe = read_count_atomic(per_thread_counters);
    // uint64_t observe = global_counter.load(std::memory_order_relaxed);

    fprintf(stdout, "%d s, sum of the counters: %" PRIu64 ".\n", i, observe);
  }

  for (int i = 0; i < COUNTERS_NUM; i++) {
    pthread_join(__thread_id_map[i], nullptr);
  }

  return 0;
}
