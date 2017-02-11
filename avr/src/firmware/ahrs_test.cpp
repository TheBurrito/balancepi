/* MPU9250 Basic Example Code
 by: Kris Winer
 date: April 1, 2014
 license: Beerware - Use this code however you'd like. If you
 find it useful you can buy me a beer some time.
 Modified by Brent Wilkins July 19, 2016

 Demonstrate basic MPU-9250 functionality including parameterizing the register
 addresses, initializing the sensor, getting properly scaled accelerometer,
 gyroscope, and magnetometer data out. Added display functions to allow display
 to on breadboard monitor. Addition of 9 DoF sensor fusion using open source
 Madgwick and Mahony filter algorithms. Sketch runs on the 3.3 V 8 MHz Pro Mini
 and the Teensy 3.1.

 SDA and SCL should have external pull-up resistors (to 3.3V).
 10k resistors are on the EMSENSR-9250 breakout board.

 Hardware setup:
 MPU9250 Breakout --------- Arduino
 VDD ---------------------- 3.3V
 VDDI --------------------- 3.3V
 SDA ----------------------- A4
 SCL ----------------------- A5
 GND ---------------------- GND
 */

#include <imu/quaternionFilters.h>
#include <imu/MPU9250.h>

#include <system/serial.h>
#include <system/i2c.h>
#include <system/timer.h>

#include <math.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#define AHRS true         // Set to false for basic data read
#define SerialDebug true  // Set to true to get Serial output for debugging

#define DEG_TO_RAD 0.017453293f
#define RAD_TO_DEG 57.295779513f

// Pin definitions
int intPin = 12;  // These can be changed, 2 and 3 are the Arduinos ext int pins
int myLed  = 13;  // Set up pin 13 led for toggling

MPU9250 myIMU;

bool led = false;

void setLED() {
    PORTB |= (1<<5);
}

void clearLED() {
    PORTB &= ~(1<<5);
}

void toggleLED() {
    led = !led;
    
    if (led) setLED();
    else clearLED();
}

void setup()
{
  initI2C();
  // TWBR = 12;  // 400 kbit/sec I2C speed
  initSerial(57600);
  initTimer();

  // Set up the interrupt pin, its set as active high, push-pull
  /*pinMode(intPin, INPUT);
  digitalWrite(intPin, LOW);
  pinMode(myLed, OUTPUT);
  digitalWrite(myLed, HIGH);*/
  
  sei();
  
  DDRB |= (1<<5); //LED pin

  // Read the WHO_AM_I register, this is a good test of communication
  uint8_t c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
  printf("MPU9250 I AM 0x%02x I should be 0x71\n", c);

  if (c == 0x71) // WHO_AM_I should always be 0x68
  {
    printf("MPU9250 is online...\n");

    // Start by performing self test and reporting values
    myIMU.MPU9250SelfTest(myIMU.SelfTest);
    printf("x-axis self test: acceleration trim within : %d(%%*10) of factory value\n", (int)(myIMU.SelfTest[0] * 10));
    printf("y-axis self test: acceleration trim within : %d(%%*10) of factory value\n", (int)(myIMU.SelfTest[1] * 10));
    printf("z-axis self test: acceleration trim within : %d(%%*10) of factory value\n", (int)(myIMU.SelfTest[2] * 10));
    printf("x-axis self test: gyration trim within : %d(%%*10) of factory value\n", (int)(myIMU.SelfTest[3] * 10));
    printf("y-axis self test: gyration trim within : %d(%%*10) of factory value\n", (int)(myIMU.SelfTest[4] * 10));
    printf("z-axis self test: gyration trim within : %d(%%*10) of factory value\n", (int)(myIMU.SelfTest[5] * 10));

    // Calibrate gyro and accelerometers, load biases in bias registers
    myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);

    myIMU.initMPU9250();
    // Initialize device for active mode read of acclerometer, gyroscope, and
    // temperature
    printf("MPU9250 initialized for active data mode....\n");

    // Read the WHO_AM_I register of the magnetometer, this is a good test of
    // communication
    uint8_t d = myIMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
    printf("AK8963 I AM 0x%02x I should be 0x48\n", d);

    // Get magnetometer calibration from AK8963 ROM
    myIMU.initAK8963(myIMU.magCalibration);
    // Initialize device for active mode read of magnetometer
    printf("AK8963 initialized for active data mode....\n");
    if (SerialDebug)
    {
      //  Serial.println("Calibration values: ");
      printf("X-Axis sensitivity adjustment value %d\n", (int)(myIMU.magCalibration[0] * 100));
      printf("Y-Axis sensitivity adjustment value %d\n", (int)(myIMU.magCalibration[1] * 100));
      printf("Z-Axis sensitivity adjustment value %d\n", (int)(myIMU.magCalibration[2] * 100));
    }
  } // if (c == 0x71)
  else
  {
    printf("Could not connect to MPU9250: 0x%02x\n", c);
    while(1) ; // Loop forever if communication doesn't happen
  }
}

void loop()
{
  // If intPin goes high, all data registers have new data
  // On interrupt, check if data ready interrupt
  if (myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  { 
    myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values
    myIMU.getAres();

    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    myIMU.ax = (float)myIMU.accelCount[0]*myIMU.aRes; // - accelBias[0];
    myIMU.ay = (float)myIMU.accelCount[1]*myIMU.aRes; // - accelBias[1];
    myIMU.az = (float)myIMU.accelCount[2]*myIMU.aRes; // - accelBias[2];

    myIMU.readGyroData(myIMU.gyroCount);  // Read the x/y/z adc values
    myIMU.getGres();

    // Calculate the gyro value into actual degrees per second
    // This depends on scale being set
    myIMU.gx = (float)myIMU.gyroCount[0]*myIMU.gRes;
    myIMU.gy = (float)myIMU.gyroCount[1]*myIMU.gRes;
    myIMU.gz = (float)myIMU.gyroCount[2]*myIMU.gRes;

    myIMU.readMagData(myIMU.magCount);  // Read the x/y/z adc values
    myIMU.getMres();
    // User environmental x-axis correction in milliGauss, should be
    // automatically calculated
    myIMU.magbias[0] = +470.;
    // User environmental x-axis correction in milliGauss TODO axis??
    myIMU.magbias[1] = +120.;
    // User environmental x-axis correction in milliGauss
    myIMU.magbias[2] = +125.;

    // Calculate the magnetometer values in milliGauss
    // Include factory calibration per data sheet and user environmental
    // corrections
    // Get actual magnetometer value, this depends on scale being set
    myIMU.mx = (float)myIMU.magCount[0]*myIMU.mRes*myIMU.magCalibration[0] -
               myIMU.magbias[0];
    myIMU.my = (float)myIMU.magCount[1]*myIMU.mRes*myIMU.magCalibration[1] -
               myIMU.magbias[1];
    myIMU.mz = (float)myIMU.magCount[2]*myIMU.mRes*myIMU.magCalibration[2] -
               myIMU.magbias[2];
   
  } // if (readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)

  // Must be called before updating quaternions!
  myIMU.updateTime();

  // Sensors x (y)-axis of the accelerometer is aligned with the y (x)-axis of
  // the magnetometer; the magnetometer z-axis (+ down) is opposite to z-axis
  // (+ up) of accelerometer and gyro! We have to make some allowance for this
  // orientationmismatch in feeding the output to the quaternion filter. For the
  // MPU-9250, we have chosen a magnetic rotation that keeps the sensor forward
  // along the x-axis just like in the LSM9DS0 sensor. This rotation can be
  // modified to allow any convenient orientation convention. This is ok by
  // aircraft orientation standards! Pass gyro rate as rad/s
//  MadgwickQuaternionUpdate(ax, ay, az, gx*PI/180.0f, gy*PI/180.0f, gz*PI/180.0f,  my,  mx, mz);
  MahonyQuaternionUpdate(myIMU.ax, myIMU.ay, myIMU.az, myIMU.gx*DEG_TO_RAD,
                         myIMU.gy*DEG_TO_RAD, myIMU.gz*DEG_TO_RAD, myIMU.my,
                         myIMU.mx, myIMU.mz, myIMU.deltat);

  if (!AHRS)
  {
    myIMU.delt_t = millis() - myIMU.count;
    
    if (myIMU.delt_t > 500)
    {
      if(SerialDebug)
      {        
        printf("accel: (%d, %d, %d)\n", (int)(1000*myIMU.ax), (int)(1000*myIMU.ay), (int)(1000*myIMU.az));
        printf("gyro: (%d, %d, %d)\n", (int)(1000*myIMU.gx), (int)(1000*myIMU.gy), (int)(1000*myIMU.gz));
        printf("mag: (%d, %d, %d)\n", (int)(1000*myIMU.mx), (int)(1000*myIMU.my), (int)(1000*myIMU.mz));

        myIMU.tempCount = myIMU.readTempData();  // Read the adc values
        // Temperature in degrees Centigrade
        myIMU.temperature = ((float) myIMU.tempCount) / 333.87 + 21.0;
        // Print temperature in degrees Centigrade
        printf("Temperature is %d C*10\n", (int)(10*myIMU.temperature));
      }

      myIMU.count = millis();
    } // if (myIMU.delt_t > 500)
  } // if (!AHRS)
  else
  {
    // Serial print and/or display at 0.5 s rate independent of data rates
    myIMU.delt_t = millis() - myIMU.count;

    // update LCD once per half-second independent of read rate
    if (myIMU.delt_t > 500)
    {
      myIMU.yaw   = atan2(2.0f * (*(getQ()+1) * *(getQ()+2) + *getQ() *
                    *(getQ()+3)), *getQ() * *getQ() + *(getQ()+1) * *(getQ()+1)
                    - *(getQ()+2) * *(getQ()+2) - *(getQ()+3) * *(getQ()+3));
      myIMU.pitch = -asin(2.0f * (*(getQ()+1) * *(getQ()+3) - *getQ() *
                    *(getQ()+2)));
      myIMU.roll  = atan2(2.0f * (*getQ() * *(getQ()+1) + *(getQ()+2) *
                    *(getQ()+3)), *getQ() * *getQ() - *(getQ()+1) * *(getQ()+1)
                    - *(getQ()+2) * *(getQ()+2) + *(getQ()+3) * *(getQ()+3));
      myIMU.pitch *= RAD_TO_DEG;
      myIMU.yaw   *= RAD_TO_DEG;
      // Declination of SparkFun Electronics (40°05'26.6"N 105°11'05.9"W) is
      // 	8° 30' E  ± 0° 21' (or 8.5°) on 2016-07-19
      // - http://www.ngdc.noaa.gov/geomag-web/#declination
      myIMU.yaw   -= 8.5;

      if(SerialDebug)
      {
        int yaw = myIMU.yaw;
        int pitch = myIMU.pitch;
        int roll = myIMU.roll;
        int rate = myIMU.sumCount / myIMU.sum;
        printf("YPR: %d, %d, %d\nrate = %d Hz\n", yaw, pitch, roll, rate);
      }

      myIMU.count = millis();
      myIMU.sumCount = 0;
      myIMU.sum = 0;
    } // if (myIMU.delt_t > 500)
  } // if (AHRS)
}

int main(void) {
    setup();
    
    while(1) {
        loop();
    }
}
