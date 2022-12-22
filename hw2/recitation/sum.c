// Copyright (c) 2012 MIT License by 6.172 Staff

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint32_t data_t;
const int U = 10000000;   // size of the array. 10 million vals ~= 40MB
const int N = 100000000;  // number of searches to perform

int main() {
  data_t* data = (data_t*) malloc(U * sizeof(data_t));
  if (data == NULL) {
    free(data);
    printf("Error: not enough memory\n");
    exit(-1);
  }

  // fill up the array with sequential (sorted) values.
  int i;
  for (i = 0; i < U; i++) {
    data[i] = i;
  }

  printf("Allocated array of size %d\n", U);
  printf("Summing %d random values...\n", N);

  data_t val = 0;
  data_t seed = 42;
  for (i = 0; i < N; i++) {
    int l = rand_r(&seed) % U;
    val = (val + data[l]);
  }

  free(data);
  printf("Done. Value = %d\n", val);
  return 0;
}

/* root@CD-DZ0104843:/home/hanbabang/learn/MIT6_172F18_hw2/recitation# valgrind --tool=cachegrind --branch-sim=yes ./sum
==7312== Cachegrind, a cache and branch-prediction profiler
==7312== Copyright (C) 2002-2017, and GNU GPL'd, by Nicholas Nethercote et al.
==7312== Using Valgrind-3.13.0 and LibVEX; rerun with -h for copyright info
==7312== Command: ./sum
==7312==
--7312-- warning: L3 cache found, using its data for the LL simulation.
Allocated array of size 10000000
Summing 100000000 random values...
Done. Value = 938895920
==7312==
==7312== I   refs:      3,440,230,598
==7312== I1  misses:            1,196
==7312== LLi misses:            1,178
==7312== I1  miss rate:          0.00%
==7312== LLi miss rate:          0.00%
==7312==
==7312== D   refs:        610,074,621  (400,058,558 rd   + 210,016,063 wr)
==7312== D1  misses:      100,548,313  ( 99,922,445 rd   +     625,868 wr)
==7312== LLd misses:       84,953,992  ( 84,328,190 rd   +     625,802 wr)
==7312== D1  miss rate:          16.5% (       25.0%     +         0.3%  )
==7312== LLd miss rate:          13.9% (       21.1%     +         0.3%  )
==7312==
==7312== LL refs:         100,549,509  ( 99,923,641 rd   +     625,868 wr)
==7312== LL misses:        84,955,170  ( 84,329,368 rd   +     625,802 wr)
==7312== LL miss rate:            2.1% (        2.2%     +         0.3%  )
==7312==
==7312== Branches:        210,044,351  (110,043,828 cond + 100,000,523 ind)
==7312== Mispredicts:           5,547  (      5,316 cond +         231 ind)
==7312== Mispred rate:            0.0% (        0.0%     +         0.0%   ) */

/* root@CD-DZ0104843:/home/hanbabang/learn/MIT6_172F18_hw2/recitation# lscpu
Architecture:        x86_64
CPU op-mode(s):      32-bit, 64-bit
Byte Order:          Little Endian
CPU(s):              8
On-line CPU(s) list: 0-7
Thread(s) per core:  2
Core(s) per socket:  4
Socket(s):           1
Vendor ID:           GenuineIntel
CPU family:          6
Model:               142
Model name:          Intel(R) Core(TM) i5-10210U CPU @ 1.60GHz
Stepping:            12
CPU MHz:             2112.005
BogoMIPS:            4224.01
Hypervisor vendor:   Microsoft
Virtualization type: full
L1d cache:           32K
L1i cache:           32K
L2 cache:            256K
L3 cache:            6144K
Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 ss ht syscall nx pdpe1gb rdtscp lm constant_tsc rep_good nopl xtopology cpuid pni pclmulqdq ssse3 fma cx16 pcid sse4_1 sse4_2 movbe popcnt aes xsave avx f16c rdrand hypervisor lahf_lm abm 3dnowprefetch invpcid_single ssbd ibrs ibpb stibp ibrs_enhanced fsgsbase bmi1 avx2 smep bmi2 erms invpcid rdseed adx smap clflushopt xsaveopt xsavec xgetbv1 xsaves flush_l1d arch_capabilities */
