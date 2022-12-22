#include <iostream>
#include <stdio.h>
#include <typeinfo>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <assert.h>

using namespace std;

int main(int argc, char* argv[]) {
    int i;
    printf("i:%d\n", i);
    {
        int j;
        printf("j:%d\n", j);
    }
    return 0;
}

// #define HASHBITS 8
// #define PHI_2_64 11400714819323198485U

// int hash_func(void* p) {
//   long x = (long)p;
//   printf("hash_func----p:%p, x:%d\n", p, x);
//   /* multiplicative hashing */
//   printf("hash_func----x * PHI_2_64:%ld\n", x * PHI_2_64); // 4512374017494110304, 以为是285,620,922,694,162,882,032,062,560
//   printf("hash_func---->> (64 - HASHBITS):%d\n", 4512374017494110304>>56);
//   int res = ((long)(x * PHI_2_64)) >> (64 - HASHBITS); // 这个地方到底应该是默认转成什么的时候出的问题
//   printf("hash_func----res:%d\n", res);

//   cout<<typeid(1).name()<<endl;
//   cout<<typeid(11400714819323198485U).name()<<endl;
//   cout<<typeid(11400714819323198485).name()<<endl;

//   return res;
// }

// mutex test_mtx1;
// mutex test_mtx2;

// mutex test_mtx[10];
// mutex log_mtx;

// void lock_fun(int i) {
//     {
//         lock_guard<mutex> lg(log_mtx);
//         test_mtx[i].lock();
//         printf("lock:%d\n", i);
//     }
// }

// void unlock_fun(int i) {
//     {
//         lock_guard<mutex> lg(log_mtx);
//         test_mtx[i].unlock();
//         printf("unlock:%d\n", i);
//     }
// }

// void fun(int i) {
//     // printf("ENTER-----------------线程:%d\n", i);
//     lock_fun(i);
//     unlock_fun(i);
// }

// int tmp = -1;

// void test(int i) {
//     // int tmp = -1;
//     cout<<"tid:"<<i<<endl;
//     tmp*=i;
//     cout<<"tmp:"<<tmp<<endl;
// }

// cout<<"hihi"<<endl;
// int a[5] = {1,2,3,4,5};
// cout<<a[0]<<endl;
// cout<<a[5]<<endl;

// void* p = (void*)0x17e46e0;
// printf("%p\n", p);
// long x = (long)p;
// printf("%ld\n", x);
// int s = hash_func(p);
// printf("%d\n", s);
