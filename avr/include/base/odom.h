#ifndef ODOM_H_
#define ODOM_H_

void initOdom(float ticksPerMLeft, float ticksPerMRight, float wheelWidth);
void updateOdom();
void getPose(float *x, float *y, float *a);
void setPose(float x, float y, float a);
void resetPosition(float *x, float *y);
void getWheelVel(float *left, float *right, float dt);

#endif //ODOM_H_
