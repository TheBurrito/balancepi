#include <util/pid.h>

float runPID(PID *pid, float value, float setpoint) {
    float error = setpoint - value;
    
    pid->sum += error * pid->i;
    
    if (pid->sum > pid->max) pid->sum = pid->max;
    else if (pid->sum < pid->min) pid->sum = pid->min;
    
    float diff = value - pid->last;
    float output = pid->p * error + pid->sum - diff * pid->d;
    
    pid->last = value;
    
    if (output > pid->max) output = pid->max;
    else if (output < pid->min) output = pid->min;
    
    return output;
}
