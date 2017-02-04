#include <output/motor.h>
#include <avr/io.h>
#include <stdlib.h>

//EnA is PWM controlled with OCR1A on PB1
//EnB is PWM controlled with OCR1B on PB2
//In1 on PD6
//In2 on PD7
//In3 on PB3
//In4 on PB4

void initMotors() {
    //setup timer1 for motor pwm output
    TCCR1A = 0b10100000; //non-inverted pwm mode, phase & freq correct, top in ICR1
    TCCR1B = 0b00010001; //no prescaler
    ICR1 = MOTOR_MAX; //set PWM resolution
    OCR1A = 0;
    OCR1B = 0;
    
    DDRD |= 0b11000000; //enable A direction outputs
    DDRB |= 0b00011110; //enable B direction outputs and pwm outputs
}

inline void setPWMA(int val) {
    OCR1A = val;
}

inline void setPWMB(int val) {
    OCR1B = val;
}

inline void setDirA(uint8_t a, uint8_t b) {
    if (a) {
        PORTD |= (1<<6);
    } else {
        PORTD &= ~(1<<6);
    }
    
    if (b) {
        PORTD |= (1<<7);
    } else {
        PORTD &= ~(1<<7);
    }
}

inline void setDirB(uint8_t a, uint8_t b) {
    if (a) {
        PORTB |= (1<<3);
    } else {
        PORTB &= ~(1<<3);
    }
    
    if (b) {
        PORTB |= (1<<4);
    } else {
        PORTB &= ~(1<<4);
    }
}

void driveA(int val) {
    uint8_t dir = val < 0;
    val = abs(val);
    if (val > MOTOR_MAX) val = MOTOR_MAX;
    
    setPWMA(val);
    
    if (dir) {
        setDirA(1, 0);
    } else {
        setDirA(0, 1);
    }
}

void driveB(int val) {
    uint8_t dir = val < 0;
    val = abs(val);
    if (val > MOTOR_MAX) val = MOTOR_MAX;
    
    setPWMB(val);
    
    if (dir) {
        setDirB(1, 0);
    } else {
        setDirB(0, 1);
    }
}

void coast(uint8_t a, uint8_t b) {
    if (a) OCR1A = 0;
    if (b) OCR1B = 0;
}

void brake(uint8_t a, uint8_t b) {
    if (a) {
        setDirA(1, 1);
        setPWMA(MOTOR_MAX);
    }
    
    if (b) {
        setDirB(1, 1);
        setPWMB(MOTOR_MAX);
    }
}
