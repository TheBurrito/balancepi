
#ifndef CIRCULARBUFFER_H_
#define CIRCULARBUFFER_H_

#include <stdint.h>

#define CREATE_BUFFER() {{0}, 0, 0}

typedef struct {
    char _buf[256];
    uint8_t _head;
    uint8_t _tail;
} CircularBuffer;

void cb_put(CircularBuffer *buf, char c);
char cb_get(CircularBuffer *buf);

uint8_t cb_isFull(CircularBuffer *buf);
uint8_t cb_isEmpty(CircularBuffer *buf);

#endif //CIRCULARBUFFER_H_
