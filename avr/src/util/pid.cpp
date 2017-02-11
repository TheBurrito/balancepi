#include <util/pid.h>

PID::PID(float kp, float ki, float kd) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
    
    _sum = _last = 0.0f;
    
    _min = 0.0f;
    _max = 100.0f;
}

PID::~PID() {
}

void PID::setRange(float min, float max) {
    _min = min;
    _max = max;
}

float PID::run(float value, float setpoint) {
    float error = setpoint - value;
    
    _sum += error * _ki;
    
    if (_sum > _max) _sum = _max;
    else if (_sum < _min) _sum = _min;
    
    float diff = value - _last;
    float output = _kp * error + _sum - diff * _kd;
    
    _last = value;
    
    if (output > _max) output = _max;
    else if (output < _min) output = _min;
    
    return output;
}
