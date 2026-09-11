#include <atomic>
#include <cstdint>
#include <ctime>
#include <fmt/core.h>
#include <fmt/format.h>
#include <mutex>
#include <shared_mutex>
#include <sys/time.h>
#include <thread>
#include <vector>

auto ClockMs() -> uint64_t {
  struct timeval tm;

  gettimeofday(&tm, nullptr);

  return static_cast<uint64_t>(tm.tv_sec * 1000) +
         static_cast<uint64_t>(tm.tv_usec / 1000);
}

static const int THREAD = 8;

struct ParallelTotalMetrics {
  uint64_t write_cnts_{0};
  uint64_t start_time_{0};
  std::mutex mutex_;

  void Begin() { start_time_ = ClockMs(); }

  void RecordWrite(uint64_t scan_write_) {
    std::unique_lock<std::mutex> lk(mutex_);

    write_cnts_ += scan_write_;
  }

  void Report() {
    auto now = ClockMs();
    auto elapsed = now - start_time_;

    auto read_per_sec = write_cnts_ / static_cast<double>(elapsed) * 1000;

    fmt::print("---- BEGIN ----\n");

    fmt::print("write: {}\n", write_cnts_);

    fmt::print("---- END ----\n");
  }
};

struct AtomicMetrics {
  uint64_t cnt_{0};
  uint64_t start_time_{0};

  uint64_t duration_ms_;
  uint64_t last_report_at_;
  std::string reporter_;
  uint64_t last_cnt_{0};

  void Tick() { cnt_++; }

  void Begin() { start_time_ = ClockMs(); }

  auto shouldFinish() -> bool { return ClockMs() - start_time_ > duration_ms_; }

  void Report() {
    // auto now = ClockMs();
    // auto elsped = now - start_time_;
    // if (elsped - last_report_at_ > 1000) {
    //   fmt::print(stderr,
    //              "[{:5.2f}] {}: total_cnt={:<10} throughput={:<10.3f} "
    //              "avg_throughput={:<10.3f}\n",
    //              elsped / 1000.0, reporter_, cnt_,
    //              (cnt_ - last_cnt_) /
    //                  static_cast<double>(elsped - last_report_at_) * 1000,
    //              cnt_ / static_cast<double>(elsped) * 1000);
    //   last_report_at_ = elsped;
    //   last_cnt_ = cnt_;
    // }
  }
};

int main() {
  std::atomic<uint64_t> atomic_a{0};
  std::mutex lock_a{};
  std::shared_mutex shared_lock_a;

  std::atomic<uint64_t> atomic_b;

  std::vector<std::thread> threads;

  uint64_t duration_ms = 10000;

  auto cas = [&atomic_a](AtomicMetrics &metrics) {
    uint64_t state;
    uint64_t new_state;
    do {
      state = atomic_a.load();
      new_state = state + 1;
    } while (!atomic_a.compare_exchange_weak(state, new_state));
  };

  auto fetch = [&atomic_a]() { atomic_a.fetch_add(1); };

  auto lock_test = [&lock_a]() {
    lock_a.lock();
    lock_a.unlock();
  };

  auto shared_lock_test = [&shared_lock_a]() {
    shared_lock_a.lock();
    shared_lock_a.unlock();
  };

  ParallelTotalMetrics total_metrics;

  total_metrics.Begin();

  for (int thread_id = 0; thread_id < THREAD; thread_id++) {
    threads.emplace_back([thread_id, &atomic_a, duration_ms, &total_metrics,
                          &cas, &fetch, &lock_test, &shared_lock_test] {
      AtomicMetrics metrics{0, 0, duration_ms};

      metrics.Begin();

      while (!metrics.shouldFinish()) {
        // cas(metrics);
        // fetch();
        // lock_test();
        shared_lock_test();
        metrics.Tick();
      }

      total_metrics.RecordWrite(metrics.cnt_);
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }

  total_metrics.Report();
}
