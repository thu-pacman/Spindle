#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//#define SHADOW_BASE 0x7fff8000

#define PRIME 1000000007UL

#define SHADOW(Mem) ((((uint64_t)Mem) >> 3) % PRIME + SHADOW_BASE)

#define OFFSET(Mem) ((((uint64_t)Mem) >> 2) & 1)

static unsigned long cnt = 0;
static unsigned long cntl = 0;

typedef struct LowShadowObj {
  void *ptr;
  void *caller;
  unsigned size;
  unsigned char is_free;
} LowShadowObj;

static uint64_t SHADOW_BASE;
static uint64_t LOW_SHADOW_OFFSET = 0;
static uint64_t LOW_SHADOW_FREE_OFFSET = 0;
static LowShadowObj *LOW_SHADOW_BASE;
static LowShadowObj *LOW_SHADOW_FREE_BASE;

uint64_t MASK[4] = {0b11, 0b1100, 0b110000, 0b11000000};

void __init_main() {
  printf("Start\n");
  SHADOW_BASE = (uint64_t)malloc(PRIME*sizeof(int)*2);
  LOW_SHADOW_BASE = (LowShadowObj *)malloc(536870912 * sizeof(LowShadowObj));
  LOW_SHADOW_FREE_BASE =
      (LowShadowObj *)malloc(536870912 * sizeof(LowShadowObj));
}

void __fini_main() {
  printf("End\n");
  long uninit = 0, useafree = 0, nofree = 0;
  nofree = (long)LOW_SHADOW_OFFSET - (long)LOW_SHADOW_FREE_OFFSET;
  // for (uint64_t i = 0; i < LOW_SHADOW_OFFSET; ++i) {
  //   void *p = LOW_SHADOW_BASE[i].ptr;
  //   char *sdp = (char *)SHADOW(p);
  //   uint8_t off = OFFSET(p);
  //   uint8_t sd = ((*sdp) >> off) & 0b1111;
  //   if (sd & 0b1100 == 0b0000)
  //     ++uninit;
  //   if (sd & 0b11 == 0b10)
  //     ++useafree;
  //   if (sd & 0b11 == 0b00)
  //     ++nofree;
  // }
  printf("Total objects: %u\n", LOW_SHADOW_OFFSET);
  printf("Uninitialized use: %u.\nUse after free: %u\nMemory leak: %u\n",
         uninit, useafree, nofree);
  printf("full: %lu\n", cnt);
  printf("loop: %lu\n", cntl);

  printf("Alloc: \n");
  for (int i = 0; i < LOW_SHADOW_OFFSET; ++i) {
    printf("%p, %p\n", LOW_SHADOW_BASE[i].ptr, LOW_SHADOW_BASE[i].caller);
  }
  printf("Free: \n");
  for (int i = 0; i < LOW_SHADOW_FREE_OFFSET; ++i) {
    printf("%p, %p\n", LOW_SHADOW_FREE_BASE[i].ptr,
           LOW_SHADOW_FREE_BASE[i].caller);
  }
  free(SHADOW_BASE);
  free(LOW_SHADOW_BASE);
  free(LOW_SHADOW_FREE_BASE);
}

void __record_free(void *p) {
  LOW_SHADOW_FREE_BASE[LOW_SHADOW_FREE_OFFSET].ptr = p;
  LOW_SHADOW_FREE_BASE[LOW_SHADOW_FREE_OFFSET].caller =
      __builtin_return_address(0);
  ;
  LOW_SHADOW_FREE_OFFSET++;
}

void __record_malloc(void *p, size_t sz) {
  LOW_SHADOW_BASE[LOW_SHADOW_OFFSET].ptr = p;
  LOW_SHADOW_BASE[LOW_SHADOW_OFFSET].caller = __builtin_return_address(0);
  LOW_SHADOW_BASE[LOW_SHADOW_OFFSET].size = sz;  LOW_SHADOW_BASE[LOW_SHADOW_OFFSET].is_free = 0;
  // LOW_SHADOW_BASE[LOW_SHADOW_OFFSET++].ptr = p;
  char *sdp = (char *)SHADOW(p);
  uint8_t off = OFFSET(p);
  uint8_t sd = 0b1000;
  (*sdp) |= (sd << off);
  unsigned *off_at_ls = sdp + sizeof(int);
  *off_at_ls = LOW_SHADOW_OFFSET;
  LOW_SHADOW_OFFSET++;
}

void __record_realloc(void *p, void *old_p, size_t sz) {
  LOW_SHADOW_FREE_BASE[LOW_SHADOW_FREE_OFFSET++].ptr = old_p;
  LOW_SHADOW_BASE[LOW_SHADOW_OFFSET].ptr = p;
  LOW_SHADOW_BASE[LOW_SHADOW_OFFSET].caller = __builtin_return_address(0);
  LOW_SHADOW_BASE[LOW_SHADOW_OFFSET].size = sz;  LOW_SHADOW_BASE[LOW_SHADOW_OFFSET].is_free = 0;
  // LOW_SHADOW_BASE[LOW_SHADOW_OFFSET++].ptr = p;
  char *sdp = (char *)SHADOW(p);
  uint8_t off = OFFSET(p);
  uint8_t sd = 0b1000;
  (*sdp) |= (sd << off);
  unsigned *off_at_ls = sdp + sizeof(int);
  *off_at_ls = LOW_SHADOW_OFFSET;
  LOW_SHADOW_OFFSET++;
  // LOW_SHADOW_BASE[LOW_SHADOW_OFFSET++].ptr = p;
  // char *sdp = (char *)SHADOW(p);
  // uint8_t off = OFFSET(p);
  // uint8_t sd = 0b1000;
  // (*sdp) |= (sd << off);
}

void __record_calloc(void *p, size_t num, size_t sz) {
  LOW_SHADOW_BASE[LOW_SHADOW_OFFSET].ptr = p;
  LOW_SHADOW_BASE[LOW_SHADOW_OFFSET].caller = __builtin_return_address(0);  LOW_SHADOW_BASE[LOW_SHADOW_OFFSET].size = sz;  LOW_SHADOW_BASE[LOW_SHADOW_OFFSET].is_free = 0;
  // LOW_SHADOW_BASE[LOW_SHADOW_OFFSET++].ptr = p;
  char *sdp = (char *)SHADOW(p);
  uint8_t off = OFFSET(p);
  uint8_t sd = 0b1000;
  (*sdp) |= (sd << off);
  unsigned *off_at_ls = sdp + sizeof(int);
  *off_at_ls = LOW_SHADOW_OFFSET;
  LOW_SHADOW_OFFSET++;
  // LOW_SHADOW_BASE[LOW_SHADOW_OFFSET++].ptr = p;
  // char *sdp = (char *)SHADOW(p);
  // uint8_t off = OFFSET(p);
  // uint8_t sd = 0b1000;
  // (*sdp) |= (sd << off);
}

void __record_load(void *p) {
  char *sdp = (char *)SHADOW(p);
  uint8_t off = OFFSET(p);
  uint8_t sd = ((*sdp) >> off) & 0b1111;
  sd &= 0b0110;
  (*sdp) &= (0b1111 << (~off));
  (*sdp) |= (sd << off);
}

void __record_store(void *p) {
  char *sdp = (char *)SHADOW(p);
  uint8_t off = OFFSET(p);
  uint8_t sd = ((*sdp) >> off) & 0b1111;
  sd &= 0b1110;
  sd |= ((sd & 0b1000) >> 1);
  (*sdp) &= (0b1111 << (~off));
  (*sdp) |= (sd << off);
}

void __record_addr(void *p) {
  ++cnt;
  // char *sdp = (char *)SHADOW(p);
  // uint8_t off = OFFSET(p);
  // uint8_t sd = ((*sdp) >> off) & 0b1111;
  // sd &= 0b1110;
  // sd |= ((sd & 0b1000) >> 1);
  // (*sdp) &= (0b1111 << (~off));
  // (*sdp) |= (sd << off);
}

void __is_in_range(void *base, void *addr) {
  ++cnt;
  char *sdp = (char *)SHADOW(base);
  int *off_at_ls = sdp + sizeof(int);
  unsigned sz = LOW_SHADOW_BASE[*off_at_ls].size;
  if (addr - base >= sz) {
    printf("%x, %x, %u, ", base, addr, sz);
    printf("Out of boundary\n");
  }
  // uint8_t off = OFFSET(p);
  // uint8_t sd = ((*sdp) >> off) & 0b1111;
  // sd &= 0b1110;
  // sd |= ((sd & 0b1000) >> 1);
  // (*sdp) &= (0b1111 << (~off));
  // (*sdp) |= (sd << off);
}

void __is_loop_in_range() {
  ++cntl;
  // char *sdp = (char *)SHADOW(p);
  // uint8_t off = OFFSET(p);
  // uint8_t sd = ((*sdp) >> off) & 0b1111;
  // sd &= 0b1110;
  // sd |= ((sd & 0b1000) >> 1);
  // (*sdp) &= (0b1111 << (~off));
  // (*sdp) |= (sd << off);
}

void __is_in_range_at_loop_end() { ++cntl; }

void __is_in_range_at_multi_loop_end() { ++cntl; }
