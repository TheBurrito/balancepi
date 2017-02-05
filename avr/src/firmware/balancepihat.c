#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <system/serial.h>
#include <system/timer.h>

#include <output/motor.h>

#include <util/pid.h>
#include <util/odom.h>

void init();

unsigned long curMillis;
unsigned long lastOdomMillis;
const uint8_t odomPeriod = 10;
uint8_t period8;

unsigned long lastDebugMillis;
const uint16_t debugPeriod = 1000;

int main(void) {
    init();
    
    while(1) {
        curMillis = millis();
        
        period8 = curMillis - lastOdomMillis;
        if (period8 >= odomPeriod) {
            //set the arduino LED if falling behind in iteration
            if (period8 > odomPeriod) {
                PORTB |= (1<<5);
            } else {
                PORTB &= ~(1<<5);
            }
            
            lastOdomMillis += odomPeriod;
            updateOdom();
        }
        
        if (curMillis - lastDebugMillis >= debugPeriod) {
            lastDebugMillis += debugPeriod;
            float x, y, a;
            getPose(&x, &y, &a);
            printf("(%d, %d, %d)\n", (int)(x * 100), (int)(y * 100), (int)(a * 180 / 3.151429));
        }
    }
}

void init() {
    initSerial(57600);
    initTimer();
    initMotors();
    initOdom(15423.85881, 15423.85881, 0.145);
    
    DDRB |= (1<<5); //set LED pin to output
    
    lastOdomMillis = lastDebugMillis = millis();
    
    sei();
}
