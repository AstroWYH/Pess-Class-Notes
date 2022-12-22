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

using namespace std;

#define START_LOG_TIME()                                                       \
    std::chrono::steady_clock::time_point t1 =                                 \
            std::chrono::steady_clock::now();                                  \
    std::chrono::steady_clock::time_point t2 = t1;                             \
    std::chrono::duration<double> time_used =                                  \
            std::chrono::duration_cast<std::chrono::duration<double>>(t2 -     \
                                                                      t1);
#define PRINT_COST_TIME(name)                                                  \
    t2 = std::chrono::steady_clock::now();                                     \
    time_used = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - \
                                                                          t1); \
    std::cout << name << "TIME COST: " << (time_used.count() * 1000)           \
              << " ms" << std::endl;                                           \
    t1 = t2;                                                                   \

atomic<int> cnt_conflict(0);
extern atomic<int> cnt_lock;

#define RUN_TIMES 1
#define TID_FOREACH_TIMES 20
#define HASHBITS 8
// #define HASHBITS 4
// #define HASHBITS 2
#define TABLESIZE (1<<HASHBITS)
#define MAX_ENTRIES (TABLESIZE-1)

typedef struct entry {
    void* ptr; // 8 bytes
    size_t size; // 8 bytes
} entry_t;

/* struct alignas(64) entry_t {
    void* ptr;
    size_t size;
}; */

/* typedef struct entry {
    void* ptr;
    size_t size;
    long tmp[6];
} entry_t; */

typedef struct hashtable_t {
    entry_t hashtable[TABLESIZE];
    int entries;
    pthread_mutex_t lock;
} hashtable_t;

/* mutex log_mtx1;
mutex log_mtx2;
mutex log_mtx3;
mutex log_mtx4; */

hashtable_t ht = {{}, 0, PTHREAD_MUTEX_INITIALIZER};

/* golden ratio (sqrt(5)-1)/2 * (2^64) */
#define PHI_2_64  11400714819323198485U

// int hash_func(void* p) {
//     long x = (long)p;
//     /* multiplicative hashing */
//     return (x * PHI_2_64) >> (64 - HASHBITS);
// }

int hash_func(void* p) {
    long x = (long)p;
    return x % 256;
}

void hashtable_lock() {
    pthread_mutex_lock(&ht.lock);
}

void hashtable_unlock() {
    pthread_mutex_unlock(&ht.lock);
}

int cnt_hashtable_insert = 0;

void hashtable_insert(void* p, int size, int i) {
    hashtable_lock();
    assert(ht.entries < TABLESIZE);
    ht.entries++;
    hashtable_unlock();

    int s = hash_func(p);
    /* open addressing with linear probing */

    do {
    if (!ht.hashtable[s].ptr) {
        if (s >= MAX_ENTRIES) {
        printf("hashtable_insert: ERROR! slot:%d entries:%d\n", s, ht.entries);
        }
        cnt_hashtable_insert++;
        ht.hashtable[s].ptr = p;
        ht.hashtable[s].size = size;
        break;
    }
    cnt_conflict++;
    /* conflict, look for next item */
    s++;
    if (s == TABLESIZE) {
        printf("hashtable_insert: slot reached TABLESIZE, reset to 0!\n");
        // s %= TABLESIZE;
    }
    } while (1);
}

void hashtable_insert_locked(void* p, int size, int tid) {
    int s = hash_func(p);
    /* s = s % TABLESIZE; */
    /* open addressing with linear probing */
    // printf("hashtable_insert_locked: tid:%d slot:%d\n", tid, s);

    hashtable_lock();
    assert(ht.entries < TABLESIZE);
    ht.entries++;
    do {
    if (!ht.hashtable[s].ptr) {
        ht.hashtable[s].ptr = p;
        ht.hashtable[s].size = size;
        // printf("hashtable_insert_locked: tid:%d finish find slot:%d\n", tid, s);
        break;
    }
    cnt_conflict++;
    /* conflict, look for next item */
    s++;
    if (s == TABLESIZE) {
        // printf("hashtable_insert_locked: slot reached TABLESIZE, reset to 0!\n");
        s %= TABLESIZE;
    }
    } while (1);
    hashtable_unlock();
}

void hashtable_insert_fair(void* p, int size, int tid) {
    hashtable_lock();
    assert(ht.entries < TABLESIZE);
    ht.entries++;
    hashtable_unlock();

    int s = hash_func(p);
    /* s = s % TABLESIZE; */
    // printf("hashtable_insert_fair: tid:%d slot:%d\n", tid, s);
    /* open addressing with linear probing */
    {
    /* lock_guard<mutex> lg(log_mtx1); */
    hashlock_lock(s);
    /* hashlock_lock(s, tid); */
    /* printf("hashtable_insert_fair 1: tid:%d get lock:%d\n", tid, s); */
    }
    do {
    if (!ht.hashtable[s].ptr) {
        ht.hashtable[s].ptr = p;
        ht.hashtable[s].size = size;
        // printf("hashtable_insert_fair: tid:%d finish find slot:%d\n", tid, s);
        break;
    }
    cnt_conflict++;
    int olds = s;
    /* conflict, look for next item */
    s++;
    if (s == TABLESIZE) {
        // printf("hashtable_insert_fair: slot reached TABLESIZE, reset to 0!\n");
        s %= TABLESIZE;
    }
    /* fair lock, hold the old lock, before grabbing the new one */
    {
        /* lock_guard<mutex> lg(log_mtx2); */
        hashlock_lock(s);
        /* hashlock_lock(s, tid); */
        /* printf("hashtable_insert_fair 2: tid:%d get lock:%d\n", tid, s); */
    }

    {
        /* lock_guard<mutex> lg(log_mtx3); */
        hashlock_unlock(olds);
        /* hashlock_unlock(olds, tid); */
        /* printf("hashtable_insert_fair 3: tid:%d get lock:%d\n", tid, s); */
    }
    } while (1);
    {
        /* lock_guard<mutex> lg(log_mtx4); */
        hashlock_unlock(s);
        /* hashlock_unlock(s, tid); */
        /* printf("hashtable_insert_fair 4: tid:%d get lock:%d\n", tid, s); */
    }
}

void hashtable_insert_lockless(void* p, int size, int tid) {
    hashtable_lock();
    assert(ht.entries < TABLESIZE);
    ht.entries++;
    hashtable_unlock();

    int s = hash_func(p);
    printf("hashtable_insert_lockless: tid:%d slot:%d\n", tid, s);
    /* open addressing with linear probing */
    do {
    if (!InterlockedCompareExchange64((uint64_t*)&ht.hashtable[s].ptr, (uint64_t)p, 0)) {
        ht.hashtable[s].size = size;
        break;
    }

    s++;
    if (s == TABLESIZE) {
        printf("hashtable_insert_fair: slot reached TABLESIZE, reset to 0!\n");
        s %= TABLESIZE;
    }
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
    if (ht.hashtable[s].size == 0) {
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
    printf("hashtable_dump\n");
    int i;
    int cnt_for = 0;
    int cnt_dump = 0;
    for (i = 0; i < TABLESIZE; i++) {
        cnt_for++;
        if (ht.hashtable[i].ptr) {
            cnt_dump++;
            printf("hashtable_dump: i:%d ptr:%p size:%ld\n", i, ht.hashtable[i].ptr, ht.hashtable[i].size);
        }
    }
    printf("hashtable_dump: for times:%d dump times:%d\n", cnt_for, cnt_dump);
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
    printf("hashtable_free\n");
    int i;
    int cnt_for = 0;
    int cnt_free = 0;
    for (i = 0; i < TABLESIZE; i++) {
        cnt_for++;
        if (ht.hashtable[i].ptr) {
            cnt_free++;
            free(ht.hashtable[i].ptr);
            ht.entries--;
        }
    }
    printf("hashtable_free: for times:%d dump times:%d\n", cnt_for, cnt_free);
}

void hashtable_fill(int n, int tid) {
    int i;
    /* printf("hashtable_fill: n:%d\n", n); */
    for (i = 0; i < 127; i++) {
    int sz = random() % 1000;
    char* p = (char*)malloc(sz);
    static char* p_constant = p;

    /* test one of these below */

    hashtable_insert(p, sz, i);
    // hashtable_insert((void*)p_constant + tid, sz, i);

    // hashtable_insert_locked(p, sz, tid);
    // hashtable_insert_fair(p, sz, tid);

    /* same keys, 10 threads * 20 ps: free&assert*/
    // hashtable_insert_locked(p_constant, sz, tid);
    // hashtable_insert_fair(p_constant, sz, tid);

    /* diff keys, 10 threads * 20 ps: hash_func*/
    // hashtable_insert_locked((void*)(long(p_constant) + (int)(tid * 20 + i)), sz, tid);
    // hashtable_insert_fair((void*)(long(p_constant) + (int)(tid * 20 + i)), sz, tid);

    /* 10 threads * 20 ps: free&assert*/
    // hashtable_insert_lockless(p, sz, tid);
    }
}

/* 10 threads * 20 */
/* same keys */
/* hashtable_insert_locked: 3ms */
/* hashtable_insert_fair: 100ms */
/* diff keys */
/* hashtable_insert_locked: 3ms */
/* hashtable_insert_fair: 3ms */

int main(int argc, char* argv[]) {
    printf("----------------------------------- main ----------------------------------- \n");
    /* 50 % fill ratio */
    int n = MAX_ENTRIES / 2;

    /* GCC sometimes gets confused by Cilk */
    /* #pragma GCC diagnostic ignored "-Wmaybe-uninitialized" */

    int threads = 1;
    if (argc > 1) {
    threads = atoi(argv[1]);
    }

    unsigned seed = time(NULL);
    if (argc > 2) {
    seed = atoi(argv[2]);
    }

    /* verify hashtable entries are aligned, otherwise even 64bit */
    /* writes/reads won't be guaranteed to be atomic */
    assert((uintptr_t)ht.hashtable % sizeof(long) == 0);
    /* 16 bytes 128 bits */

    srandom(seed);
// #define VERBOSE
#ifdef VERBOSE
    printf("VERBOSE: seed = %d\n", seed);
    printf("VERBOSE: sizeof entry = %ld\n", sizeof(entry_t)); /* 16 bytes 128 bits */
    printf("VERBOSE: &entries = %p &last=%p\n", &ht.entries, &ht.hashtable[TABLESIZE]);
#endif

/* -----------------------------------CILK----------------------------------- */
#ifdef CILK
    int i;
    for (i = 1; i < threads; i++) {
    cilk_spawn hashtable_fill(n / threads);
    }
#endif

    /* hashtable_fill(n / threads); */

#ifdef CILK
    cilk_sync;
#endif

/* -----------------------------------MUL_THREAD----------------------------------- */
    START_LOG_TIME();
    for (int i = 0; i < RUN_TIMES; i++) {
#define MUL_THREAD
#ifdef MUL_THREAD
        vector<shared_ptr<thread>> vec_tid;
        for (int i = 0; i < threads; i++) {
            vec_tid.push_back(make_shared<thread>(hashtable_fill, n / threads, i));
        }

        for (int i = 0; i < threads; i++) {
            vec_tid.at(i)->join();
        }
#endif

#define VERY_VERBOSE
#ifdef VERY_VERBOSE
        // hashtable_dump();
#endif
        printf("start: %d entries\n", ht.entries);
        hashtable_free();
        printf("end: %d entries\n", ht.entries);
        assert(ht.entries == 0);
        cout<<"conflict times:"<<cnt_conflict<<endl;
        cout<<"cnt_lock:"<<cnt_lock<<endl;
    }
    PRINT_COST_TIME("");

    return 0;
}