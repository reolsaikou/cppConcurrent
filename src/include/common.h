#pragma once

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>

#define COUNTERS_NUM 8
#define CACHE_LINE_SIZE 64

#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
#define READ_ONCE(x)                                                           \
  ({                                                                           \
    typeof(x) ___x = ACCESS_ONCE(x);                                           \
    ___x;                                                                      \
  })

#define WRITE_ONCE(x, val)                                                     \
  do {                                                                         \
    ACCESS_ONCE(x) = (val);                                                    \
  } while (0)

#define DEFINE_PER_THREAD(type, name)                                          \
  struct {                                                                     \
    alignas(CACHE_LINE_SIZE) type v{};                                         \
  } __per_thread_##name[COUNTERS_NUM];

#define __THREAD_ID_MAP_EMPTY 0

#ifndef SUBOBJ
pthread_key_t thread_id_key;
#else
extern pthread_key_t thread_id_key;
#endif

#ifndef SUBOBJ
pthread_t __thread_id_map[COUNTERS_NUM];
pthread_mutex_t __thread_id_map_lock;
#else
extern pthread_t __thread_id_map[COUNTERS_NUM];
extern pthread_mutex_t __thread_id_map_lock;
#endif

typedef pthread_mutex_t spinlock_t;

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

static inline void spin_lock(spinlock_t *sp) {
  int en;

  en = pthread_mutex_lock(sp);
  if (en != 0) {
    fprintf(stderr, "pthread mutex lock: %s\n", strerror(en));
    perror("spin lock:pthread mutex lock");
    abort();
  }
}

static inline void spin_unlock(spinlock_t *sp) {
  int en;

  en = pthread_mutex_unlock(sp);

  if (en != 0) {
    fprintf(stderr, "pthread mutex unlock: %s\n", strerror(en));
    abort();
  }
}

static inline int __smp_thread_id() {
  auto tid = pthread_self();

  for (int i = 0; i < COUNTERS_NUM; i++) {

    if (__thread_id_map[i] == tid) {
      int64_t v = i + 1;

      if (pthread_setspecific(thread_id_key, (void *)v) != 0) {
        perror("pthread setspecific");
        exit(EXIT_FAILURE);
      }
      return i;
    }
  }
  spin_lock(&__thread_id_map_lock);

  for (int i = 0; i < COUNTERS_NUM; i++) {
    if (__thread_id_map[i] == tid) {
      spin_unlock(&__thread_id_map_lock);
      return i;
    }
  }

  spin_unlock(&__thread_id_map_lock);

  fprintf(stderr, "smp thread id: Rogue thread, id: %d(%#x)\n", (int)tid);
  exit(EXIT_FAILURE);
}

static inline int smp_thread_id(pthread_key_t pthread_key) {

  auto *id = (pthread_getspecific(pthread_key));

  if (id == nullptr) {
    return __smp_thread_id();
  }
  return reinterpret_cast<long>(id) - 1;
}
