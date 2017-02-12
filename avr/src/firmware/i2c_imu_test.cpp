#include <avr/io.h>
#include <avr/interrupt.h>

#include <system/timer.h>

#include <system/i2c.h>

#include <util/delay.h>

#define MPU9250_ADDR 0x68

#define MPU9250_R_WHOAMI 0x75

#define MAG_ADDR 0x0c
#define MAG_WHOAMI 0x00

void init();

unsigned long curMillis;

unsigned long lastDebugMillis;
const uint16_t debugPeriod = 500;

uint8_t led = 0;

void toggleLED() {
    led = !led;
    
    if (led)
        PORTB |= (1<<5);
    else
        PORTB &= ~(1<<5);
}

int main(void) {
    init();
    
    while(1) {
        curMillis = millis();
        
        if (curMillis - lastDebugMillis >= debugPeriod) {
            lastDebugMillis += debugPeriod;
            
            toggleLED();
        }
    }
}

void init() {
    initTimer();
    
    i2c.initMaster();
    
    DDRB |= (1<<5); //set LED pin to output
    PORTB = 0;
    
    sei();
    
    uint8_t whoami = 0;
    
    i2c.write(MPU9250_ADDR, 0x6B, 0x00, true);
    _delay_ms(100);
    
    i2c.write(MPU9250_ADDR, 0x6B, 0x01, true);
    _delay_ms(200);
    
    i2c.write(MPU9250_ADDR, 0x37, 0x02, true);
    _delay_ms(100);
    
    i2c.read(MAG_ADDR, MAG_WHOAMI, whoami, true);
    if (whoami != 0x48) {
        PORTB |= (1<<5);
        while(1);
    }
    
    whoami = 0;
    
    i2c.read(MPU9250_ADDR, MPU9250_R_WHOAMI, whoami, true);
    if (whoami != 0x71) {
        PORTB |= (1<<5);
        while(1);
    }
}
