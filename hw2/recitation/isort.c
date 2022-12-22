// Copyright (c) 2012 MIT License by 6.172 Staff

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Typedefs */

typedef uint32_t data_t;

extern void quickSortIterative(data_t arr[], int l, int h);

/* Insertion sort */
void isort(data_t* left, data_t* right) {
  data_t* cur = left + 1;
  while (cur <= right) {
    data_t val = *cur;
    data_t* index = cur - 1;

    while (index >= left && *index > val) {
      *(index + 1) = *index;
      index--;
    }

    *(index + 1) = val;
    cur++;
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Error: wrong number of arguments.\n");
    exit(-1);
  }
  int N = atoi(argv[1]);
  int K = atoi(argv[2]);
  unsigned int seed = 42;
  printf("Sorting %d values...\n", N);
  data_t* data = (data_t*) malloc(N * sizeof(data_t));
  if (data == NULL) {
    free(data);
    printf("Error: not enough memory\n");
    exit(-1);
  }

  int i, j;
  for (j = 0; j < K ; j++) {
    for (i = 0; i < N; i++) {
      data[i] = rand_r(&seed);
      //  printf("%d ", data[i]);
    }
    //  printf("\n");

    isort(data, data + N - 1);
    //quickSortIterative(data, 0, N);
    /*for (i = 0; i < N; i++) {
      printf("%d ", data[i]);
    }
    printf("\n");*/
  }
  free(data);
  printf("Done!\n");
  return 0;
}

/* root@CD-DZ0104843:/home/hanbabang/learn/MIT6_172F18_hw2/recitation# perf record ./isort 10000 10
Sorting 10000 values...
Done!
[ perf record: Woken up 1 times to write data ]
[ perf record: Captured and wrote 0.101 MB perf.data (2626 samples) ]
root@CD-DZ0104843:/home/hanbabang/learn/MIT6_172F18_hw2/recitation# perf report
# To display the perf.data header info, please use --header/--header-only options.
#
#
# Total Lost Samples: 0
#
# Samples: 2K of event 'cpu-clock:pppH'
# Event count (approx.): 656500000
#
# Overhead  Command  Shared Object      Symbol
# ........  .......  .................  .............................
#
    99.54%  isort    isort              [.] isort
     0.27%  isort    [kernel.kallsyms]  [k] __softirqentry_text_start
     0.08%  isort    libc-2.27.so       [.] rand_r
     0.04%  isort    [kernel.kallsyms]  [k] __lock_text_start
     0.04%  isort    isort              [.] main
     0.04%  isort    libc-2.27.so       [.] 0x00000000000452dc


#
# (Tip: Skip collecting build-id when recording: perf record -B) */