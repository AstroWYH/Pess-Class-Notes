/* Copyright (c) 2013 MIT License by 6.172 Staff
 *
 * DON'T USE THE FOLLOWING SOFTWARE, IT HAS KNOWN BUGS, AND POSSIBLY
 * UNKNOWN BUGS AS WELL.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "./hashlock.h"
#include <pthread.h>
#include <thread>
#include <vector>
#include "./common.h"
#include <chrono>
#include <iostream>
#include <atomic>
#include <mutex>
#include <cilk/cilk.h>

using namespace std;

#define START_LOG_TIME()                                                   \
    std::chrono::steady_clock::time_point t1 =                             \
            std::chrono::steady_clock::now();                              \
    std::chrono::steady_clock::time_point t2 = t1;                         \
    std::chrono::duration<double> time_used =                              \
            std::chrono::duration_cast<std::chrono::duration<double>>(t2 - \
                                                                      t1);
#define PRINT_COST_TIME(name)                                                  \
    t2 = std::chrono::steady_clock::now();                                     \
    time_used = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - \
                                                                          t1); \
    std::cout << name << "TIME COST: " << (time_used.count() * 1000)           \
              << " ms." << std::endl;                                          \
    t1 = t2;                                                                   \

atomic<int> cnt_conflict(0);
extern atomic<int> cnt_lock;

mutex log_mtx1;
mutex log_mtx2;
mutex log_mtx3;
mutex log_mtx4;

#define HASHBITS 8
#define TABLESIZE (1<<HASHBITS) // 256
// #define TABLESIZE 16 // 16
// #define TABLESIZE 4 // 4

#define MAX_ENTRIES (TABLESIZE-1) // 255

// typedef struct entry {
//   void* ptr; // 8字节 64位
//   size_t size; // 8字节 64位
// } entry_t;

struct alignas(64) entry_t {
  void* ptr; // 8字节 64位
  size_t size; // 8字节 64位
};

// typedef struct entry {
//   void* ptr; // 8字节 64位
//   size_t size; // 8字节 64位
//   long tmp[6];
// } entry_t;

typedef struct hashtable_t {
  entry_t hashtable[TABLESIZE];
  int entries;
  pthread_mutex_t lock;
} hashtable_t;

hashtable_t ht = {{}, 0, PTHREAD_MUTEX_INITIALIZER};

/* golden ratio (sqrt(5)-1)/2 * (2^64) */
#define PHI_2_64  11400714819323198485U

// int hash_func(void* p) {
//   long x = (long)p;
//   /* multiplicative hashing */
//   return (x * PHI_2_64) >> (64 - HASHBITS);
// }

// 测试key全不同时，需要用这个hash
int hash_func(void* p) {
  long x = (long)p;
  return x % 256;
}

int cnt_hashtable_insert = 0;

void hashtable_insert(void* p, int size, int i) {
  assert(ht.entries < TABLESIZE); // 256
  ht.entries++; // 这个地方如果多线程，可能也会导致++次数没那么多，比如只加119次，多线程bug1，但实际上进来了120次

  int s = hash_func(p);
//   printf("----hashtable_insert---- i:%d, size:%d, p:%p, s:%d\n", i, size, p, hash_func(p));

  if (s >= 255) {
    printf("hashtable_insert: COMPUTE! s:%d p:%p en:%d\n", s, p, ht.entries); // 经测试，s经过hash_func计算，最大为255
  }
  /* open addressing with linear probing */

  do {
    if (s == 255) {
    //   printf("hashtable_insert: when s=255! s:%d ptr_256:%p en:%d\n", s, ht.hashtable[256].ptr, ht.entries);
    }
    if (s == 256) {
    //   printf("hashtable_insert:  when s=256! s:%d ptr_256:%p en:%d\n", s, ht.hashtable[256].ptr, ht.entries);
    }
    if (!ht.hashtable[s].ptr) { // 如果该slot位置的ptr为null
      if (s >= 255) {
        printf("hashtable_insert: ENTER! s:%d en:%d\n", s, ht.entries);
      }
      cnt_hashtable_insert++; // 这个值打印出来还是127，说明有相同的ptr被覆盖了？
      ht.hashtable[s].ptr = p;
      ht.hashtable[s].size = size;
      break;
    }
    cnt_conflict++;
    /* conflict, look for next item */
    s++;
    if (s == 256) {
        printf("hashtable_insert: s到256了！重新置0！\n");
        // s = 0;
    }
  } while (1);
}

void hashtable_lock() {
  pthread_mutex_lock(&ht.lock);
}

void hashtable_unlock() {
  pthread_mutex_unlock(&ht.lock);
}

void hashtable_insert_locked(void* p, int size, int thread_id) {
  int s = hash_func(p);
//   s = s % 16; // TABLESIZE为16时新增
  /* open addressing with linear probing */
//   printf("hashtable_insert_locked: 线程%d 插入的slot:%d\n", thread_id, s);

  hashtable_lock();
  assert(ht.entries < TABLESIZE);
  ht.entries++;
  do {
    if (!ht.hashtable[s].ptr) {
      ht.hashtable[s].ptr = p;
      ht.hashtable[s].size = size;
    //   printf("hashtable_insert_locked: ENTER! 线程%d 找到了自己的位置：%d\n", thread_id, s);
      break;
    }
    // printf("hashtable_insert_locked冲突了????????????: 线程%d 插入的slot:%d\n", thread_id, s);
    cnt_conflict++;
    /* conflict, look for next item */
    s++;
    if (s == TABLESIZE) { // 256 16
        // printf("hashtable_insert_locked: s到256了！重新置0！\n");
        s = 0;
    }
  } while (1);
  hashtable_unlock();
}

void hashtable_insert_fair(void* p, int size, int thread_id) {
  hashtable_lock();
//   printf("hashtable_insert_fair: -----------------ht.entries:%d\n", ht.entries);
  assert(ht.entries < TABLESIZE);
  ht.entries++;
  hashtable_unlock();

  int s = hash_func(p);
//   s = s % 16; // TABLESIZE为16时新增
//   s = s % 4;
//   printf("hashtable_insert_fair: -----------------线程%d 插入的slot:%d\n", thread_id, s);
  /* open addressing with linear probing */
  {
    // lock_guard<mutex> lg(log_mtx1);
    hashlock_lock(s); // 对于多线程，s是相同的值（冲突），和s是不同的值（无冲突），这是两种情况
    // hashlock_lock(s, thread_id);
    // printf("hashtable_insert_fair 1: 线程%d 拿到了锁%d\n", thread_id, s);
  }
  do {
    if (!ht.hashtable[s].ptr) {
      ht.hashtable[s].ptr = p;
      ht.hashtable[s].size = size;
    //   printf("hashtable_insert_fair: ENTER! 线程%d 找到了自己的位置：%d\n", thread_id, s);
      break;
    }
    cnt_conflict++;
    int olds = s;
    /* conflict, look for next item */
    s++;
    if (s == TABLESIZE) { // 256 16
        // printf("hashtable_insert_fair: s到256了！重新置0！\n");
        s = 0;
    }
    /* fair lock, hold the old lock, before grabbing the new one */ // 公平的体现
    {
        // lock_guard<mutex> lg(log_mtx2);
        hashlock_lock(s);
        // hashlock_lock(s, thread_id);
        // printf("hashtable_insert_fair 2: 线程%d 拿到了锁%d\n", thread_id, s);
    }

    {
        // lock_guard<mutex> lg(log_mtx3);
        hashlock_unlock(olds);
        // hashlock_unlock(olds, thread_id);
        // printf("hashtable_insert_fair 3: 线程%d 释放了锁%d\n", thread_id, olds);
    }
  } while (1);
  {
    //   lock_guard<mutex> lg(log_mtx4);
      hashlock_unlock(s);
    //   hashlock_unlock(s, thread_id);
    //   printf("hashtable_insert_fair 4: 线程%d 释放了锁%d\n", thread_id, s);
  }
}

void hashtable_insert_lockless(void* p, int size, int tid) {
  hashtable_lock();
  assert(ht.entries < TABLESIZE);
  ht.entries++;
  hashtable_unlock();

  int s = hash_func(p);
  printf("hashtable_insert_lockless: -----------------线程%d 插入的slot:%d\n", tid, s);
  /* open addressing with linear probing */
  do {
    // if (!ht.hashtable[s].ptr) {
    //   ht.hashtable[s].ptr = p;
    //   ht.hashtable[s].size = size;
    //   break;
    // }

    // printf("dbg 1\n");
    uint64_t old_ptr = 0;
    uint64_t old_size = 0;
    // printf("dbg 2\n");
    uint64_t res_ptr = InterlockedCompareExchange64((uint64_t*)&ht.hashtable[s].ptr, (uint64_t)p, (uint64_t)old_ptr);
    if (res_ptr == 0) {
        InterlockedCompareExchange64((uint64_t*)&ht.hashtable[s].size, (uint64_t)size, (uint64_t)old_size);
        break;
    }

    // printf("dbg 3\n");
    // uint64_t old_size = 0;
    // InterlockedCompareExchange64((uint64_t*)&ht.hashtable[s].size, (uint64_t)size, (uint64_t)old_size);
    // printf("dbg 4\n");

    // ht.hashtable[s].ptr = p;
    // ht.hashtable[s].size = size;
    /* conflict, look for next item */
    s++;
    s %= TABLESIZE;
  } while (1);
}

entry_t* hashtable_lookup(void* p) {
  size_t s = hash_func(p);
  do {
    if (p == ht.hashtable[s].ptr) {
      return &ht.hashtable[s];
    }
    if (ht.hashtable[s].ptr == NULL) {
      /* not found */
      return NULL;
    }
    if (ht.hashtable[s].size == 0) { // size=0说明malloc就失败了，说明什么？之前多线程分配内存的时候，有p分配失败，分配到0的情况？
      /* race on incomplete size, pretend not found */
      return NULL;
    }
    /* check next */
    s++;
#ifndef STUDENT_VERSION
    s %= TABLESIZE;
#endif
  } while (1);
}

void hashtable_dump(void) {
  printf("----hashtable_dump----\n");
  int i;
  int cnt_for = 0;
  int cnt_dump = 0;
  for (i = 0; i < TABLESIZE; i++) {
    cnt_for++;
    if (ht.hashtable[i].ptr) {
      cnt_dump++;
      printf("hashtable_dump: %d %p %ld\n", i, ht.hashtable[i].ptr, ht.hashtable[i].size);
    }
  }
  printf("hashtable_dump: 遍历次数:%d dump次数:%d\n", cnt_for, cnt_dump);
}

void hashtable_entries(void) {
  int i;
  int count = 0;
  for (i = 0; i < TABLESIZE; i++) {
    if (ht.hashtable[i].ptr) {
      count++;
    }
  }
}

void hashtable_test(int n) {
  int i;
  for (i = 0; i < n; i++) {
    int s = random() & (TABLESIZE - 1);
    void* p = ht.hashtable[s].ptr;
    void* pl = hashtable_lookup(p);
    assert(p == pl);
  }
}

void hashtable_free(void) {
  printf("----hashtable_free----\n");
  int i;
  int cnt_for = 0;
  int cnt_free = 0;
  for (i = 0; i < TABLESIZE; i++) {
    cnt_for++;
    if (ht.hashtable[i].ptr) { // 如果该slot位置的ptr不为null
      cnt_free++;
      free(ht.hashtable[i].ptr);
      ht.entries--;
    }
  }
  printf("hashtable_free: 遍历次数:%d 释放次数:%d\n", cnt_for, cnt_free);
}

// char* p_global = nullptr;
// volatile atomic<int> cnt_slot(0);

// volatile int cnt_slot = -1;
// mutex cnt_mtx;
// mutex add_mtx;

void hashtable_fill(int n, int thread_id) {
  int i;
//   printf("----hashtable_fill---- n:%d\n", n); // 127 / 12
  for (i = 0; i < 127; i++) { // 决定每个线程插入多少个 1 / 20 / n
    // {
    //     lock_guard<mutex> lg(cnt_mtx);
    //     cnt_slot++;
    //     printf("----hashtable_fill---- 线程:%d cnt_slot:%d\n", thread_id, (int)cnt_slot);
    // }
    int sz = random() % 1000;
    char* p = (char*)malloc(sz);
    static char* p_global = p;
    // printf("----hashtable_fill---- p_global:%p\n", p_global);
    printf("----hashtable_fill---- i:%d, p:%p, sz:%d, slot:%d\n", i, p, sz, hash_func(p));
    // test one of these below
    // hashtable_insert((void*)p_global + thread_id, sz, i);
    hashtable_insert(p, sz, i);
    // hashtable_insert_locked(p, sz, thread_id);
    // hashtable_insert_fair(p, sz, thread_id);
    // hashtable_insert_locked(p_global, sz, thread_id); // key完全相同 对于10个线程 * 20
    // hashtable_insert_fair(p_global, sz, thread_id); // key完全相同
    // hashtable_insert_locked((void*)p_global + thread_id, sz, thread_id); // key完全不同 对于10个线程 * 1
    // hashtable_insert_fair((void*)p_global + thread_id, sz, thread_id); // key完全不同

    // void* p_tmp = nullptr;
    // {
    //     lock_guard<mutex> lg(add_mtx);
    //     p_tmp = (void*)(long(p_global) + cnt_slot);
    // }

    // 这样A、B线程都进来，A的cnt_slot为0，B的cnt_slot为1，此时cnt_slot已经为1，AB在这加都是加1，所以不行！
    // 应该用一个自带身份，且永远不会变的标志。
    // hashtable_insert_locked((void*)(long(p_global) + (int)(thread_id * 20 + i)), sz, thread_id); // key完全不同 对于10个线程 * 20
    // hashtable_insert_fair((void*)(long(p_global) + (int)(thread_id * 20 + i)), sz, thread_id); // key完全不同
    // hashtable_insert_lockless(p, sz, thread_id);
  }
//   printf("hashtable_fill: ht.entries:%d\n", ht.entries);
//   printf("hashtable_insert: ptr赋值实际次数:%d\n", cnt_hashtable_insert);
}

// 10个线程 * 1
// 1 1 0.6 0.6

// 10个线程 * 20
// 3 100ms(不打log) 2 5

// 最后两组key不同的测试，更换hashlock和entry 64位对齐没区别

int main(int argc, char* argv[]) {
  printf("---------------------------------------------------------------------- \n");
  int n = MAX_ENTRIES / 2;     /* 50 % fill ratio */ // n = 127

  // GCC sometimes gets confused by Cilk
//  #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

  int threads = 1;
  if (argc > 1) {
    threads = atoi(argv[1]);
  }

  unsigned seed = time(NULL);
  if (argc > 2) {
    seed = atoi(argv[2]);
  }

  /* verify hashtable entries are aligned, otherwise even 64bit
   * writes/reads won't be guaranteed to be atomic
   */
  assert((uintptr_t)ht.hashtable % sizeof(long) == 0);
  // 是指entry_t是64位的倍数？经打印是16字节 128位

  srandom(seed);
#define VERBOSE
#ifdef VERBOSE
  printf("VERBOSE: seed = %d\n", seed);
  printf("VERBOSE: sizeof entry = %ld\n", sizeof(entry_t)); // 16字节
  printf("VERBOSE: &entries = %p &last=%p slot_256=%p\n",
    &ht.entries, &ht.hashtable[TABLESIZE], ht.hashtable[TABLESIZE].ptr);
#endif

#ifdef CILK
  int i;
  for (i = 0; i < threads; i++) {
    cilk_spawn hashtable_fill(n / threads, i);
  }
#endif

//   hashtable_fill(n / threads);

#ifdef CILK
  cilk_sync;
#endif

  START_LOG_TIME();

for (int i = 0; i < 1; i++) {
    // #define MUL_THREAD
    #ifdef MUL_THREAD
        vector<shared_ptr<thread>> vec_ptr;
        for (int i = 0; i < threads; i++) {
            vec_ptr.push_back(make_shared<thread>(hashtable_fill, n / threads, i)); // 12
            // vec_ptr.push_back(make_shared<thread>(hashtable_fill, 1, i));
        }

        for (int i = 0; i < threads; i++) {
            vec_ptr.at(i)->join();
        }
    #endif

    #define VERY_VERBOSE
    #ifdef VERY_VERBOSE
    printf("----VERY_VERBOSE---- \n");
    //   hashtable_dump();
    #endif
    printf("main A: %d entries\n", ht.entries);
    hashtable_free();
    printf("main B: %d entries\n", ht.entries);
    assert(ht.entries == 0);
    cout<<"conflict times:"<<cnt_conflict<<endl;
    cout<<"cnt_lock:"<<cnt_lock<<endl;
}

  PRINT_COST_TIME("");

  return 0;
}

// 单线程hashtable_insert的bug
// 1）位置如果大于255，则继续给257、258等非法位置赋值，最后无法free
// 2）256位置已经占用，会跳过，这是为什么？

// 多线程hashtable_insert的两种bug
// 1）entry++没加到256次，只加到了255次，但其实256个位置都填满了，所以free的时候entry--减到了-1
// VERBOSE: seed = 1668572059
// VERBOSE: sizeof entry = 16
// VERBOSE: &entries = 0x564594409040 &last=0x564594409040 slot_256=(nil)
// hashtable_insert: ENTER! s:255 en:172
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// ----VERY_VERBOSE---- 
// main A: 255 entries
// ----hashtable_free----
// hashtable_free: 遍历次数:256 释放次数:256
// main B: -1 entries
// hashtable: hashtable.cpp:335: int main(int, char**): Assertion `ht.entries == 0' failed.
// bash: line 1: 29858 Aborted                 /root/workspace/MIT6_172F18_hw9/hashtable 10
// ---------------------------------------------------------------------- 
// 2）2个线程同时进入了if (!ht.hashtable[s].ptr)条件，导致里面一个位置赋值了，该位置又被另一个线程覆盖了，所以最后只释放255次
// VERBOSE: seed = 1668572059
// VERBOSE: sizeof entry = 16
// VERBOSE: &entries = 0x55c188409040 &last=0x55c188409040 slot_256=(nil)
// hashtable_insert: COMPUTE! s:255 p:0x7f53b800a020 en:66
// hashtable_insert: ENTER! s:255 en:66
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// hashtable_insert: s到256了！重新置0！
// ----VERY_VERBOSE---- 
// main A: 256 entries
// ----hashtable_free----
// hashtable_free: 遍历次数:256 释放次数:255
// main B: 1 entries
// hashtable: hashtable.cpp:335: int main(int, char**): Assertion `ht.entries == 0' failed.
// bash: line 1:  4370 Aborted                 /root/workspace/MIT6_172F18_hw9/hashtable 10
// ---------------------------------------------------------------------- 
// write-up 2 hashtable_insert_fair引入，安全是安全的，但冲突比较多的时候，反而更慢；冲突比较少的时候，就比hashtable_insert_locked快
// hashtable_insert_fair(p_global, sz, thread_id); 对于key全一样的情况
// conflict times:7140
// Time Cost: TIME COST: 14.1026 ms.
// hashtable_insert_locked(p_global, sz); 对于key全一样的情况
// conflict times:7140
// Time Cost: TIME COST: 0.9551 ms.
// VERBOSE: seed = 1668598260
// VERBOSE: sizeof entry = 16
// VERBOSE: &entries = 0x556fc2408140 &last=0x556fc2408140 slot_256=(nil)
// hashtable_insert_fair 1: 线程0 拿到了锁12
// hashtable_insert_fair 4: 线程0 释放了锁12
// hashtable_insert_fair 1: 线程1 拿到了锁0
// hashtable_insert_fair 4: 线程1 释放了锁0
// hashtable_insert_fair 1: 线程3 拿到了锁13
// hashtable_insert_fair 4: 线程3 释放了锁13
// hashtable_insert_fair 1: 线程2 拿到了锁5
// hashtable_insert_fair 4: 线程2 释放了锁5
// hashtable_insert_fair 1: 线程5 拿到了锁0
// hashtable_insert_fair 2: 线程5 拿到了锁1
// hashtable_insert_fair 3: 线程5 释放了锁0
// hashtable_insert_fair 4: 线程5 释放了锁1
// hashtable_insert_fair 1: 线程6 拿到了锁14
// hashtable_insert_fair 4: 线程6 释放了锁14
// hashtable_insert_fair 1: 线程4 拿到了锁10
// hashtable_insert_fair 4: 线程4 释放了锁10
// hashtable_insert_fair 1: 线程7 拿到了锁12
// hashtable_insert_fair 2: 线程7 拿到了锁13
// hashtable_insert_fair 3: 线程7 释放了锁12
// hashtable_insert_fair 2: 线程7 拿到了锁14
// hashtable_insert_fair 3: 线程7 释放了锁13
// hashtable_insert_fair 2: 线程7 拿到了锁15
// hashtable_insert_fair 3: 线程7 释放了锁14
// hashtable_insert_fair 4: 线程7 释放了锁15
// hashtable_insert_fair 1: 线程8 拿到了锁4
// hashtable_insert_fair 4: 线程8 释放了锁4
// hashtable_insert_fair 1: 线程9 拿到了锁15
// hashtable_insert: s到256了！重新置0！
// hashtable_insert_fair 2: 线程9 拿到了锁0
// hashtable_insert_fair 3: 线程9 释放了锁15
// hashtable_insert_fair 2: 线程9 拿到了锁1
// hashtable_insert_fair 3: 线程9 释放了锁0
// hashtable_insert_fair 2: 线程9 拿到了锁2
// hashtable_insert_fair 3: 线程9 释放了锁1
// hashtable_insert_fair 4: 线程9 释放了锁2
// ----VERY_VERBOSE---- 
// main A: 10 entries
// ----hashtable_free----
// hashtable_free: 遍历次数:16 释放次数:10
// main B: 0 entries