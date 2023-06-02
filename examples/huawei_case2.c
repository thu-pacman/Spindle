#include <stdint.h>

struct SampleStruct1;

typedef struct {
    uint16_t s1;
    uint16_t s2;
    uint8_t s3;
    uint8_t s4;
    uint8_t s5;
    uint8_t s6;
} SampleStruct2;

struct SampleStruct3;

struct SampleStruct3 *func2(uint8_t, uint16_t, uint8_t);
void func3(struct SampleStruct1 *, struct SampleStruct3 *);

void func1(struct SampleStruct1 *p1, void *msg, uint32_t len) {
    // check if len is valid
    SampleStruct2 *p2 = (SampleStruct2 *)msg;
    struct SampleStruct3 *p3 = func2(p2->s4, p2->s2, p2->s3);
    func3(p1, p3);
}
