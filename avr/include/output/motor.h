#ifndef MOTOR_H_
#define MOTOR_H_

#include <stdint.h>

#define MOTOR_MAX 0x7fff

void initMotors();
void driveA(int val);
void driveB(int val);
void coast(uint8_t a, uint8_t b);
void brake(uint8_t a, uint8_t b);

#endif //MOTOR_H_
