#ifndef PID_H_
#define PID_H_

class PID {
public:
    PID(float kp, float ki, float kd);
    ~PID();
    
    void setRange(float min, float max);
    
    float run(float value, float setpoint);
    
private:
    float _kp;
    float _ki;
    float _kd;
    
    float _min;
    float _max;
    
    float _sum;
    float _last;
};

#endif //PID_H_
