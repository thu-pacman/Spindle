//
//  sdetector_lib.c
//  sdetector_lib
//
//  Created by 陈源涌 on 2020/5/18.
//  Copyright © 2020 Yeenyeong. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//  The library is modified to cover 64GB of memory
//  used by some application.

#define SD_SIZE ( 1 << 31 )

#define SHADOW(addr) ( \
    (((uint64_t)(addr)) >> 3) % SD_SIZE \
)

#define OFFSET(addr) ( \
    (((uint64_t)(addr)) >>3) & 0b11 \
)

typedef struct {
    uint8_t sd;
    uint32_t idx[4];
} ShadowMemory; // 20 bytes

typedef struct {
    void* ptr;
    void* caller;
    uint32_t size;
    uint8_t is_free;
} LowShadowObj; // 24 bytes

#define BLK_SZIE ( 1 << 20 )
//  A LowShdowBlk covering 1MB memory in which is
//  stored address that is allocated
typedef struct LowShadowBlk{
    LowShadowObj obj[BLK_SZIE];
    struct LowShadowBlk* next;
} LowShadowBlk;

//  ====================================================
//  Functions to maintain a list of LowShadowBlk

static LowShadowBlk* __new_list() {
    LowShadowBlk* base = (LowShadowBlk*)malloc(sizeof(LowShadowBlk));
    base->next = NULL;
    return base;
}

#define NEW_BLOCK(ptr)  \
ptr = (LowShadowBlk*)malloc(sizeof(LowShadowBlk)); \
ptr->next = NULL;

static void __delete_list(LowShadowBlk* base) {
    while (base != NULL) {
        LowShadowBlk* tmp = base;
        base = base->next;
        free(tmp);
        tmp = NULL;
    }
}

#define DELETE_BLOCKS(ptr)          \
while (ptr != NULL) {               \
    LowShadowBlk* next = ptr->next; \
    free(ptr);                      \
    ptr = next;                     \
}

static
LowShadowObj* __index_to_pos(LowShadowBlk* base, uint32_t idx) {
    while (idx >= BLK_SZIE) {
        idx -= BLK_SZIE;
        base = base->next;
    }
    return &(base->obj[idx]);
}

#define FIND(ptr, base, idx)      \
LowShadowBlk* __b__ = (base);     \
uint32_t __i__ = (idx);           \
while (__i__ >= BLK_SIZE) {       \
    __i__ -= BLK_SZIE;            \
    __b__ = __b__->next;          \
}                                 \
ptr = &(__b__->obj[__i__]);

static
LowShadowObj* __get_end(LowShadowBlk* base, uint32_t offset) {
    while (base->next != NULL) {
        offset -= BLK_SZIE;
        base = base->next;
    }
    if (offset >= BLK_SZIE) {
        base->next = (LowShadowBlk*)malloc(sizeof(LowShadowBlk));
        base->next->next = NULL;
        base = base->next;
        offset = 0;
    }
    
    return &(base->obj[offset]);
}

#define GET_END(ptr, base, offset)                              \
uint32_t __off__ = (offset);                                    \
LowShadowBlk* ___b___ = (base);                                 \
while (___b___->next != NULL) {                                 \
    __off__ -= BLK_SZIE;                                        \
    ___b___ = ___b___->next;                                    \
}                                                               \
if (__off__ >= BLK_SIZE) {                                      \
    ___b___->next = (LowShadowBlk*)malloc(sizeof(LowShadowBlk));\
    ___b___ = ___b___->next;                                    \
    ___b___->next = NULL;                                       \
    __off__ = 0;                                                \
}                                                               \
ptr = &(___b___->obj[__off__]);


//  ====================================================

static ShadowMemory* SHADOW_BASE;
static uint32_t low_shadow_offset = 0;
static uint32_t low_shadow_free_offset = 0;
static LowShadowBlk *LOW_SHADOW_BASE;
static LowShadowBlk *LOW_SHADOW_FREE_BASE;

void __init_main() {
    printf("Start\n");
    SHADOW_BASE = (ShadowMemory*)malloc(SD_SIZE*sizeof(ShadowMemory));
    LOW_SHADOW_BASE = __new_list();
    LOW_SHADOW_FREE_BASE = __new_list();
}

void __fini_main() {
    printf("End\n");
    
    
    free(SHADOW_BASE);
    __delete_list(LOW_SHADOW_BASE);
    __delete_list(LOW_SHADOW_FREE_BASE);
}

void __record_free(void* p) {
    LowShadowObj* obj = __get_end(LOW_SHADOW_FREE_BASE, low_shadow_free_offset);
    obj->ptr = p;
    obj->caller = __builtin_return_address(0);
    ++ low_shadow_free_offset;
}

void __record_malloc(void* p, size_t sz) {
    LowShadowObj* obj = __get_end(LOW_SHADOW_BASE, low_shadow_offset);
    obj->ptr = p;
    obj->caller = __builtin_return_address(0);
    obj->size = (uint32_t)sz;
    obj->is_free = 0;
    
    uint64_t sd_idx = SHADOW(p);
    uint8_t off = (uint8_t)OFFSET(p);
    uint8_t mask = 0b01;
    SHADOW_BASE[sd_idx].sd |= (mask << (off*2));
    SHADOW_BASE[sd_idx].idx[off] = low_shadow_offset;
    ++ low_shadow_offset;
}

void __record_realloc(void* p, void* old_p, size_t sz) {
    LowShadowObj* obj = __get_end(LOW_SHADOW_FREE_BASE, low_shadow_free_offset);
    ++ low_shadow_free_offset;
    obj->ptr = old_p;
    
    obj = __get_end(LOW_SHADOW_BASE, low_shadow_offset);
    obj->ptr = p;
    obj->caller = __builtin_return_address(0);
    obj->size = (uint32_t)sz;
    obj->is_free = 0;
    
    uint64_t sd_idx = SHADOW(p);
    uint8_t off = (uint8_t)OFFSET(p);
    uint8_t mask = 0b01;
    SHADOW_BASE[sd_idx].sd |= (mask << (off*2));
    SHADOW_BASE[sd_idx].idx[off] = low_shadow_offset;
    ++ low_shadow_offset;
}

void __record_calloc(void* p, size_t num, size_t sz) {
    LowShadowObj* obj = __get_end(LOW_SHADOW_BASE, low_shadow_offset);
    obj->ptr = p;
    obj->caller = __builtin_return_address(0);
    obj->size = (uint32_t)sz;
    obj->is_free = 0;
    
    uint64_t sd_idx = SHADOW(p);
    uint8_t off = (uint8_t)OFFSET(p);
    uint8_t mask = 0b01;
    SHADOW_BASE[sd_idx].sd |= (mask << (off*2));
    SHADOW_BASE[sd_idx].idx[off] = low_shadow_offset;
    ++ low_shadow_offset;
}

void __record_load(void* p) {
    uint64_t sd_idx = SHADOW(p);
    uint8_t off = (uint8_t)OFFSET(p);
}

void __record_store(void* p) {
    
}

void __record_addr(void *p) {
    
}

void __is_in_range(void* base, void *addr) {
    uint64_t sd_idx = SHADOW(base);
    uint8_t off = (uint8_t)OFFSET(base);
    uint32_t idx = SHADOW_BASE[sd_idx].idx[off];
    
    LowShadowObj* obj = __index_to_pos(LOW_SHADOW_BASE, idx);
    uint32_t sz = obj->size;
    if (addr - base >= sz) {
        printf("%x, %x, %u", base, addr, sz);
        printf("Out of boundary\n");
    }
}

void __is_loop_in_range() {
    
}

void __is_in_range_at_loop_end(void* base, void* addr) {
    uint64_t sd_idx = SHADOW(base);
    uint8_t off = (uint8_t)OFFSET(base);
    uint32_t idx = SHADOW_BASE[sd_idx].idx[off];
    
    LowShadowObj* obj = __index_to_pos(LOW_SHADOW_BASE, idx);
    uint32_t sz = obj->size;
    if (addr - base >= sz) {
        printf("%x, %x, %u", base, addr, sz);
        printf("Out of boundadry at loop-end\n");
    }
}

void __is_in_range_at_multi_loop_end(void* base, void* addr) {
    uint64_t sd_idx = SHADOW(base);
    uint8_t off = (uint8_t)OFFSET(base);
    uint32_t idx = SHADOW_BASE[sd_idx].idx[off];
    
    LowShadowObj* obj = __index_to_pos(LOW_SHADOW_BASE, idx);
    uint32_t sz = obj->size;
    if (addr - base >= sz) {
        printf("%x, %x, %u", base, addr, sz);
        printf("Out of boundadry at loop-end\n");
    }
}
