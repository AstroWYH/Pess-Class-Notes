// Copyright (c) 2012 MIT License by 6.172 Staff

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char * argv[]) {  // What is the type of argv? 指针数组
  int i = 5;
  // The & operator here gets the address of i and stores it into pi
  int * pi = &i;
  // The * operator here dereferences pi and stores the value -- 5 --
  // into j.
  int j = *pi;

  char c[] = "6.172";
  char * pc = c;  // Valid assignment: c acts like a pointer to c[0] here.
  char d = *pc;
  printf("char d = %c\n", d);  // What does this print? char d = 6

  // compound types are read right to left in C.
  // pcp is a pointer to a pointer to a char, meaning that
  // pcp stores the address of a char pointer.
  char ** pcp;
  pcp = argv;  // Why is this assignment valid?

  const char * pcc = c;  // pcc is a pointer to char constant
  char const * pcc2 = c;  // What is the type of pcc2? 两种都表示指针可以改，指向物不能改

  // For each of the following, why is the assignment:
  // *pcc = '7';  // invalid? 指向物不能改
  pcc = *pcp;  // valid?
  pcc = argv[0];  // valid?

  char * const cp = c;  // cp is a const pointer to char 表示指针不能改，指向物可以改
  // For each of the following, why is the assignment:
  // cp = *pcp;  // invalid? 指针不能改
  // cp = *argv;  // invalid?
  *cp = '!';  // valid?

  const char * const cpc = c;  // cpc is a const pointer to char const 两者都不能改
  // For each of the following, why is the assignment:
  // cpc = *pcp;  // invalid?
  // cpc = argv[0];  // invalid?
  // *cpc = '@';  // invalid?

  return 0;
}
