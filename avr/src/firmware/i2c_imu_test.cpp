#include <avr/io.h>
#include <avr/interrupt.h>

#include <system/serial.h>
#include <system/timer.h>

#include <system/i2c.h>

#define MPU9250_ADDR 0x68

#define MPU9250_R_WHOAMI 0x75

#define MAG_ADDR 0x0c
#define MAG_WHOAMI 0x00


void init();

unsigned long curMillis;

unsigned long lastDebugMillis;
const uint16_t debugPeriod = 1000;

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
    initSerial(57600);
    initTimer();
    initI2C();
    
    DDRB |= (1<<5); //set LED pin to output
    PORTB = 0;
    
    sei();
    
    uint8_t status;
    uint8_t whoami;
    
    uint8_t mst_dis[2] = {106, (0<<5)};
    
    printf("\n");
    
    status = i2c_write(MPU9250_ADDR, mst_dis, 2);
    printf("*0: %x\n", status);
    
    status = i2c_write(MPU9250_ADDR, MPU9250_R_WHOAMI);
    printf("*1: %x\n", status);
    
    status = i2c_read(MPU9250_ADDR, &whoami);
    printf("*2: %x\n", status);
    printf("*3: %x\n", whoami);
    
    status = i2c_write(MAG_ADDR, MAG_WHOAMI);
    printf("*4: %x\n", status);
    
    status = i2c_read(MAG_ADDR, &whoami);
    printf("*5: %x\n", status);
    printf("*6: %x\n", whoami);
}
