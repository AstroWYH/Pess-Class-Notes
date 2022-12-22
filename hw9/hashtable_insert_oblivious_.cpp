#include <iostream>
#include <stdio.h>
#include <typeinfo>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <assert.h>

using namespace std;

#define TABLESIZE 256
#define THREADS 10

struct ListNode {
    int ptr;
    int size;
    ListNode* next;
    ListNode() : ptr(0), size(0), next(nullptr) {}
    ListNode(int p, int s) : ptr(p), size(s), next(nullptr) {}
    ListNode(int p, int s, ListNode* n) : ptr(p), size(s), next(n) {}
};

typedef struct hashtable_t {
    ListNode* head_node[TABLESIZE];
    int entries;
    pthread_mutex_t lock;
} hashtable_t;

int hash_func(int ptr) {
    return ptr / 10;
}

void is_greater(int ptr, ListNode* cur_node, bool& res) {
    assert(cur_node);
    res = ptr > cur_node->ptr;
}

hashtable_t ht = {{}, 0, PTHREAD_MUTEX_INITIALIZER};

void hashtable_lock() {
  pthread_mutex_lock(&ht.lock);
}

void hashtable_unlock() {
  pthread_mutex_unlock(&ht.lock);
}

void hash_insert(int ptr, int size, int tid) {
    hashtable_lock();
    assert(ht.entries < TABLESIZE);
    ht.entries++;

    cout<<"Insert "<<"key:"<<ptr<<" val:"<<size<<endl;
    int slot = hash_func(ptr);
    if (ht.head_node[slot] == nullptr) {
        ht.head_node[slot] = new ListNode(ptr, size);
    } else {
        bool res = true;
        ListNode* cur_node = ht.head_node[slot];
        ListNode* pre_node = nullptr;
        while (cur_node) {
            is_greater(ptr, cur_node, res);
            if (!res) break;
            pre_node = cur_node;
            cur_node = cur_node->next;
        }
        ListNode* ptr_node = new ListNode(ptr, size);
        if (res) {
            pre_node->next = ptr_node;
        } else {
            if (pre_node) {
                ptr_node->next = cur_node;
                pre_node->next = ptr_node;
            } else {
                ptr_node->next = cur_node;
                ht.head_node[slot] = ptr_node;
            }
        }
    }
    hashtable_unlock();
}

int main(int argc, char* argv[]) {
    // hash_insert(13, 100);
    // hash_insert(15, 100);
    // hash_insert(11, 100);

    int group = 0;
    if (argc > 1) {
        group = atoi(argv[1]);
    }

    vector<shared_ptr<thread>> vec_ptr;
    for (int i = 0; i < THREADS; i++) {
        vec_ptr.push_back(make_shared<thread>(hash_insert, group * 10 + i, 100, i));
    }

    for (int i = 0; i < THREADS; i++) {
        vec_ptr.at(i)->join();
    }

    cout<<endl;
    ListNode* cur = ht.head_node[group];
    while (cur) {
        cout<<"Dump   "<<"key:"<<cur->ptr<<" val:"<<cur->size<<endl;
        cur = cur->next;
    }

    return 0;
}