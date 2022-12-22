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

#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <mutex>
#include <atomic>

using namespace std;

#define HLOCKS 16
/* #define HLOCKS 256 */
pthread_mutex_t hashlock[HLOCKS];
#define HASHLOCK_SLOT(l) &hashlock[(l) & (HLOCKS - 1)]

/* struct alignas(64) my_lock {
    pthread_mutex_t my_hash_lock;
} */

/* custom hashlock: my_lock */
/* my_lock[HLOCKS]; */
/* #define MY_HASHLOCK_SLOT(l) &(my_lock[(l) & (HLOCKS - 1)].my_hash_lock) */

atomic<int> cnt_lock(0);

void hashlock_init() {
  int i;
  pthread_mutexattr_t recursive;
  pthread_mutexattr_init(&recursive);
  pthread_mutexattr_settype(&recursive, PTHREAD_MUTEX_RECURSIVE);
  for (i = 0; i < HLOCKS; i++) {
    pthread_mutex_init(&hashlock[i], &recursive);
  }
}

/* mutex test_mtx[HLOCKS];
mutex log_mtx_lock;
mutex log_mtx_unlock;
mutex log_mtx; */

#define UNIT_TEST

void hashlock_lock(int l) {
    pthread_mutex_lock(HASHLOCK_SLOT(l));
    // printf("lock %d\n", l);
    cnt_lock++; 
}

void hashlock_unlock(int l) {
    pthread_mutex_unlock(HASHLOCK_SLOT(l));
    // printf("unlock %d\n", l);
}

/* void hashlock_lock(int l) {
    pthread_mutex_lock(MY_HASHLOCK_SLOT(l));
    printf("lock %d\n", l);
    cnt_lock++; 
}

void hashlock_unlock(int l) {
    pthread_mutex_unlock(MY_HASHLOCK_SLOT(l));
    printf("unlock %d\n", l);
} */

/* void hashlock_lock(int l, int tid) {
    lock_guard<mutex> lg(log_mtx);
    test_mtx[l%16].lock();
    printf("tid:%d lock %d\n", tid, l);
}

void hashlock_unlock(int l, int tid) {
    lock_guard<mutex> lg(log_mtx);
    test_mtx[l%16].unlock();
    printf("tid:%d unlock %d\n", tid, l);
} */

#ifdef UNIT_TEST
/* #define HASHLOCK_MAIN */
#ifdef HASHLOCK_MAIN
int main() {
    hashlock_init();
    printf("lock hashlock size %ld\n", sizeof(hashlock[0])); // 40
    printf("lock my_lock size %ld\n", sizeof(my_lock[0])); // 64

    hashlock_lock(2);
    // hashlock_lock(2);
    hashlock_lock(5);
    hashlock_unlock(2);
    hashlock_unlock(5);
    return 0;
}
#endif
#endif
