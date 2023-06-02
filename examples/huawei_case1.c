#include <stdint.h>
#include <assert.h>

typedef struct {
    uint16_t s1;
    uint16_t s2;
    uint8_t s3;
    uint8_t s4;
    uint8_t s5;
    uint8_t s6;
} SampleStruct;

uint32_t func(void *msg, uint32_t len) {
    if (len < sizeof(SampleStruct)) {
        assert(0);
    }
    SampleStruct *pointer = (SampleStruct *)msg;
    return pointer->s1;
}