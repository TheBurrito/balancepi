#include <util/circularbuffer.h>

void cb_put(CircularBuffer *buf, char c) {
    buf->_buf[buf->_head++] = c;
    
    if (buf->_head == buf->_tail)
        buf->_tail++;
}

char cb_get(CircularBuffer *buf) {
    if (buf->_tail == buf->_head)
        return 0xFF;
    
    return buf->_buf[buf->_tail++];
}

uint8_t cb_isFull(CircularBuffer *buf) {
    return ((buf->_head + 1) == buf->_tail);
}

uint8_t cb_isEmpty(CircularBuffer *buf) {
    return (buf->_head == buf->_tail);
}
