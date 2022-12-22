#ifndef HASHLOCK_H
#define HASHLOCK_H

void hashlock_lock(int l);
void hashlock_unlock(int l);

void hashlock_lock(int l, int tid);
void hashlock_unlock(int l, int tid);

#endif
