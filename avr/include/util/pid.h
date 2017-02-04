#ifndef PID_H_
#define PID_H_

typedef struct {
    float p;
    float i;
    float d;
    float sum;
    float last;
    float min;
    float max;
} PID;

float runPID(PID *pid, float value, float setpoint);

#endif //PID_H_
