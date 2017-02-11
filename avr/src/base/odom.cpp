#include <base/odom.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

uint8_t port, a, b;
volatile unsigned int encL = 0, encR = 0;
unsigned int lastL = 0, lastR = 0;

float ticksPerMeterL, ticksPerMeterR, width;
float dL, dR;

float _x, _y, _a;

ISR(INT0_vect) {
    port = PIND;
    a = (port & (1<<2)) > 0;
    b = (port & (1<<4)) > 0;
    
    if (a == b) {
        encL--;
    } else {
        encL++;
    }
}

ISR(INT1_vect) {
    port = PIND;
    a = (port & (1<<3)) > 0;
    b = (port & (1<<5)) > 0;
    
    if (a == b) {
        encR--;
    } else {
        encR++;
    }
}

void initOdom(float ticksPerMLeft, float ticksPerMRight, float wheelWidth) {
    EICRA |= 5; //set both external interrupts to level change
    EIMSK |= 3; //enable both external interrupts
    
    ticksPerMeterL = ticksPerMLeft;
    ticksPerMeterR = ticksPerMRight;
    width = wheelWidth;
    
    _x = _y = _a = 0.0;
}

void updateOdom() {
    int diffL, diffR;
    unsigned int curL, curR;
    
    float d, dA;
    
    cli();
    curL = encL;
    sei();
    
    diffL = curL - lastL;
    dL = diffL / ticksPerMeterL;
    
    cli();
    curR = encR;
    sei();
    
    diffR = curR - lastR;
    dR = diffR / ticksPerMeterR;
    
    d = (dL + dR) / 2.0;
    dA = (dR - dL) / width;
    
    _x += d * cos(_a + dA / 2);
    _y += d * sin(_a + dA / 2);
    _a += dA;
    
    if (_a > M_PI) _a -= 2 * M_PI;
    else if (_a < -M_PI) _a += 2 * M_PI;
    
    lastL = curL;
    lastR = curR;
}

void getPose(float *x, float *y, float *a) {
    *x = _x;
    *y = _y;
    *a = _a;
}

void setPose(float x, float y, float a) {
    _x = x;
    _y = y;
    _a = a;
}

void resetPosition(float *x, float *y) {
    if (x) *x = _x;
    if (y) *y = _y;
    
    _x = _y = 0;
}

void getWheelVel(float *left, float *right, float dt) {
    *left = dL / dt;
    *right = dR / dt;
}
