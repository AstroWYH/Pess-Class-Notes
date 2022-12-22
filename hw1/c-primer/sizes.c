// Copyright (c) 2012 MIT License by 6.172 Staff

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main() {
  // Please print the sizes of the following types:
  // int, short, long, char, float, double, unsigned int, long long
  // uint8_t, uint16_t, uint32_t, and uint64_t, uint_fast8_t,
  // uint_fast16_t, uintmax_t, intmax_t, __int128, and student

  // Here's how to show the size of one type. See if you can define a macro
  // to avoid copy pasting this code.

  #define PRINT_SIZE_Arg1(type) \
    printf("size of %s : %zu bytes \n", #type, sizeof(type))

  #define PRINT_SIZE_Arg2(type_str, type) \
    printf("size of %s : %zu bytes \n", type_str, sizeof(type))

  // printf("size of %s : %zu bytes \n", "int", sizeof(int));
  // e.g. PRINT_SIZE("int", int);
  //      PRINT_SIZE("short", short);

  PRINT_SIZE_Arg2("int", int);
  PRINT_SIZE_Arg2("short", short);
  PRINT_SIZE_Arg2("long", long);
  PRINT_SIZE_Arg2("char", char);
  PRINT_SIZE_Arg2("float", float);
  PRINT_SIZE_Arg2("double", double);
  PRINT_SIZE_Arg2("unsigned int", unsigned int);
  PRINT_SIZE_Arg2("long long", long long);
  PRINT_SIZE_Arg2("uint8_t", uint8_t);
  PRINT_SIZE_Arg2("uint16_t", uint16_t);
  PRINT_SIZE_Arg2("uint32_t", uint32_t);
  PRINT_SIZE_Arg2("uint64_t", uint64_t);
  PRINT_SIZE_Arg2("uint_fast8_t", uint_fast8_t);
  PRINT_SIZE_Arg2("uint_fast16_t", uint_fast16_t);
  PRINT_SIZE_Arg2("uintmax_t", uintmax_t);
  PRINT_SIZE_Arg2("intmax_t", intmax_t);
  PRINT_SIZE_Arg2("__int128", __int128);

  PRINT_SIZE_Arg1(int*);
  PRINT_SIZE_Arg1(short*);
  PRINT_SIZE_Arg1(long*);
  PRINT_SIZE_Arg1(char*);
  PRINT_SIZE_Arg1(float*);
  PRINT_SIZE_Arg1(double*);
  PRINT_SIZE_Arg1(unsigned int*);
  PRINT_SIZE_Arg1(long long*);
  PRINT_SIZE_Arg1(uint8_t*);
  PRINT_SIZE_Arg1(uint16_t*);
  PRINT_SIZE_Arg1(uint32_t*);
  PRINT_SIZE_Arg1(uint64_t*);
  PRINT_SIZE_Arg1(uint_fast8_t*);
  PRINT_SIZE_Arg1(uint_fast16_t*);
  PRINT_SIZE_Arg1(uintmax_t*);
  PRINT_SIZE_Arg1(intmax_t*);
  PRINT_SIZE_Arg1(__int128*);

  // Alternatively, you can use stringification
  // (https://gcc.gnu.org/onlinedocs/cpp/Stringification.html) so that
  // you can write
  // e.g. PRINT_SIZE_Arg2(int);
  //      PRINT_SIZE_Arg2(short);

  // Composite types have sizes too.
  typedef struct {
    int id;
    int year;
  } student;

  // You can just use your macro here instead: PRINT_SIZE_Arg2("student", you);
  // printf("size of %s : %zu bytes \n", "student", sizeof(you));
  student you;
  you.id = 12345;
  you.year = 4;
  // PRINT_SIZE_Arg2("student", you); // 可以打变量名，也可以直接打印类型
  PRINT_SIZE_Arg1(student*);
  PRINT_SIZE_Arg1(student);

  // Array declaration. Use your macro to print the size of this.
  int x[5];
  PRINT_SIZE_Arg2("x", x); // 整个数组大小打印20bytes
  PRINT_SIZE_Arg1(&x); // 首地址打印8bytes

  return 0;
}

/* size of int : 4 bytes 
size of short : 2 bytes
size of long : 8 bytes
size of char : 1 bytes
size of float : 4 bytes
size of double : 8 bytes
size of unsigned int : 4 bytes
size of long long : 8 bytes
size of uint8_t : 1 bytes
size of uint16_t : 2 bytes
size of uint32_t : 4 bytes
size of uint64_t : 8 bytes
size of uint_fast8_t : 1 bytes
size of uint_fast16_t : 8 bytes
size of uintmax_t : 8 bytes
size of intmax_t : 8 bytes
size of __int128 : 16 bytes
size of int* : 8 bytes
size of short* : 8 bytes
size of long* : 8 bytes
size of char* : 8 bytes
size of float* : 8 bytes
size of double* : 8 bytes
size of unsigned int* : 8 bytes
size of long long* : 8 bytes
size of uint8_t* : 8 bytes
size of uint16_t* : 8 bytes
size of uint32_t* : 8 bytes
size of uint64_t* : 8 bytes
size of uint_fast8_t* : 8 bytes
size of uint_fast16_t* : 8 bytes
size of uintmax_t* : 8 bytes
size of intmax_t* : 8 bytes
size of __int128* : 8 bytes
size of student* : 8 bytes
size of student : 8 bytes
size of x : 20 bytes
size of &x : 8 bytes
 */
