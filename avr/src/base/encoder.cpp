#include <base/encoder.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdio.h>

#define NUM_INTERRUPTS 2

Encoder encA(0, &PORTD, 4);
Encoder encB(1, &PORTD, 5);

volatile int32_t _enc[NUM_INTERRUPTS];
volatile uint8_t *_bPort[NUM_INTERRUPTS];
volatile uint8_t _bPin[NUM_INTERRUPTS];

uint8_t port;
bool a, b;

ISR(INT0_vect) {
    port = *(_bPort[0]);
    a = (PORTD & (1<<2)) > 0;
    b = (port & (1<<_bPin[0])) > 0;
    
    if (a == b) {
        _enc[0]--;
    } else {
        _enc[0]++;
    }
}

ISR(INT1_vect) {
    port = *_bPort[1];
    a = (PORTD & (1<<3)) > 0;
    b = (port & (1<<_bPin[1])) > 0;
    
    if (a == b) {
        _enc[1]--;
    } else {
        _enc[1]++;
    }
}

Encoder::Encoder(uint8_t interrupt, volatile uint8_t *bPort, uint8_t bPin) {
    _int = interrupt;
    
    uint8_t oldSreg = SREG;
    cli();
    
    _enc[_int] = 0;
    _bPort[_int] = bPort;
    _bPin[_int] = bPin;
    
    SREG = oldSreg;
}

Encoder::~Encoder() {
}

int32_t Encoder::get() {
    uint8_t oldSreg = SREG;
    cli();
    int32_t count = _enc[_int];
    SREG = oldSreg;
    
    return count;
}

int32_t Encoder::getAndClear() {
    uint8_t oldSreg = SREG;
    cli();
    int32_t count = _enc[_int];
    _enc[_int] = 0;
    SREG = oldSreg;
    
    return count;
}
