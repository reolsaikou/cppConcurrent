#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <pthread.h>

#define COUNTERS_NUM 8
#define CACHE_LINE_SIZE 64

#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
#define READ_ONCE(x) ({ typeof(x) ___x = ACCESS_ONCE(x); })

#define WRITE_ONCE(x)                                                          \
  do {                                                                         \
    ACCESS_ONCE(x) = (val);                                                    \
  } while (0)

#define DEFINE_PER_THREAD(type, name)                                          \
  struct {                                                                     \
    alignas(CACHE_LINE_SIZE) type v{};                                         \
  } __per_thread_##name[COUNTERS_NUM];

#define __THREAD_ID_MAP_EMPTY 0

pthread_t __thread_id_map[COUNTERS_NUM];
pthread_mutex_t __thread_id_map_lock;

static inline int __smp_thread_id() {
  int i;
  auto tid = pthread_self();

  for (int i = 0; i < COUNTERS_NUM; i++) {

    if (__thread_id_map[i] == tid) {
      int64_t v = i + 1;

      if (pthread_setspecific(, const void *pointer))
    }
  }
}

static inline int smp_thread_id(pthread_key_t pthread_key) {
  void *id;
  id = pthread_getspecific(pthread_key);

  if (id == nullptr) {
    return
  }
}

static inline void spin_lock_init(pthread_mutex_t *sp) {
  int ret;

retry:
  ret = pthread_mutex_init(sp, nullptr);
  if (ret) {
    if (ret == EINTR) {
      goto retry;
      fprintf(stderr, "spin_lock_init:pthread_mutex_init %d\n", ret);
      abort();
    }
  }
}
