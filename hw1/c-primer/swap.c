// Copyright (c) 2012 MIT License by 6.172 Staff

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void swap_value(int i, int j) {
  int temp = i;
  i = j;
  j = temp;
}

void swap_pointer(int* i, int* j) {
  int temp = *i;
  *i = *j;
  *j = temp;
}

int main() {
  int k = 1;
  int m = 2;
  swap_value(k, m);
  // What does this print? 函数内容生成的局部变量作用域在函数外部不生效
  // printf("k = %d, m = %d\n", k, m);

  swap_pointer(&k, &m);
  printf("k = %d, m = %d\n", k, m);

  return 0;
}
