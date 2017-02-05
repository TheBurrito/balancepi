//these are the include files. They are outside the project folder
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <system/serial.h>
#include <system/timer.h>

#include <output/motor.h>

#include <util/pid.h>

volatile uint8_t a, b;
volatile unsigned int encA = 0, encB = 0;

ISR(INT0_vect) {
    uint8_t port = PIND;
    uint8_t a = (port & (1<<2)) > 0;
    uint8_t b = (port & (1<<4)) > 0;
    
    if (a == b) {
        encA--;
    } else {
        encA++;
    }
}

ISR(INT1_vect) {
    uint8_t port = PIND;
    uint8_t a = (port & (1<<3)) > 0;
    uint8_t b = (port & (1<<5)) > 0;
    
    if (a == b) {
        encB--;
    } else {
        encB++;
    }
}


void init() {
    EICRA |= 5; //set both external interrupts to level change
    EIMSK |= 3; //enable both external interrupts
    
    DDRB |= (1<<5); //set PORTB5 to output (arduino pin 13 - LED)
    
    initTimer(); //setup Timer0 for counting milliseconds
    initMotors();
    initSerial(57600);
    
    sei();
}

int main(void) {
    init();
    
    int curEncA, curEncB, lastEncA = 0, lastEncB = 0;
    
    PID pidA;
    pidA.p = 60000.0;
    pidA.i = 1000.0;
    pidA.d = 1000.0;
    pidA.sum = 0.0;
    pidA.last = 0.0;
    pidA.min = -MOTOR_MAX;
    pidA.max = MOTOR_MAX;
    
    PID pidB;
    pidB.p = 60000.0;
    pidB.i = 1000.0;
    pidB.d = 1000.0;
    pidB.sum = 0.0;
    pidB.last = 0.0;
    pidB.min = -MOTOR_MAX;
    pidB.max = MOTOR_MAX;
    
    float setSpeed = 0.0; // m/s
    
    unsigned long odomMillis = millis();
    float outA = 0.0, outB = 0.0;
    
    brake(1, 0);
    uint8_t period;
    
    while(1) {
        unsigned long curMillis = millis();
        period = curMillis - odomMillis;
        if (period >= 10) {
            if (period > 10) PORTB |= (1<<5);
            else PORTB &= ~(1<<5);
                
            odomMillis += 10;
            cli();
            curEncA = (int)encA;
            sei();
            
            int diffA = curEncA - lastEncA;
            float spdA = diffA / 154.24; // m/sec
            
            outA = runPID(&pidA, spdA, setSpeed);
            
            cli();
            curEncB = (int)encB;
            sei();
            
            int diffB = curEncB - lastEncB;
            float spdB = diffB / 154.24;
            
            outB = runPID(&pidB, spdB, setSpeed);
            
            driveA(outA);
            driveB(outB);
        
            //printf("(%d, %d)\n", (int)out, (int)(spdA * 1000));
            lastEncA = curEncA;
            lastEncB = curEncB;
        }
    }
}
