#include <avr/io.h>
#include <avr/interrupt.h>

#include <system/serial.h>
#include <system/timer.h>
#include <base/encoder.h>


void init();

unsigned long curMillis;

unsigned long lastOdomMillis;
const uint8_t odomPeriod = 10;

unsigned long lastDebugMillis;
const uint16_t debugPeriod = 1000;

int main(void) {
    init();
    
    while(1) {
        curMillis = millis();
        
        if (curMillis - lastOdomMillis >= odomPeriod) {
            lastOdomMillis += odomPeriod;
            
        }
        
        if (curMillis - lastDebugMillis >= debugPeriod) {
            lastDebugMillis += debugPeriod;
            
            printf("(%d, %d)\n", (int)encA.getAndClear(), (int)encB.getAndClear());
        }
    }
}

void init() {
    initSerial(57600);
    initTimer();
    
    DDRB |= (1<<5); //set LED pin to output
    EICRA |= 5; //set both external interrupts to level change
    EIMSK |= 3; //enable both external interrupts
    
    lastOdomMillis = lastDebugMillis = millis();
    
    sei();
}
