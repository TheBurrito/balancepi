#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

//provided as a default value when something doesn't get specified properly
#define I2C_UNKNOWN     0x00

//This is the return you want! As far as we can tell, everything worked great
#define I2C_SUCCESS     0x01

//Likely points to an implementation error as an illegal operation was attempted
#define I2C_BUSERROR    0x02

//Returned when an interrupt driven callback is properly setup.
//The specified callback function will be invoked with one of the other return
//status values to signify success or an error
#define I2C_CALLBACK    0x03

//The operation failed due to not receiving an Ack during slave addressing
#define I2C_ADDRNACK    0x04

//The operation failed due to an unexpected Nack during data transfer
#define I2C_DATANACK    0x05

//The interface is busy, likely do to an already pending callback operation
#define I2C_BUSY        0x06

//returned when trying to call a blocking operation from within a callback
//when in a callback only callback versions of the operations can be used
#define I2C_NOBLOCK     0x07

//arbitration was lost
#define I2C_LOSTARB     0x08

//the function definition for callbacks which are used with interrupt driven
//communication functions
typedef void (*i2c_complete)(uint8_t status);

class I2C {
public:
    //#TODO: setup parameterizing registers and bits for easier porting
    //and supporting multiple I2C busses per chip via various constructors
    I2C();
    
    //Init setups the control registers and sets up the bus speed
    void initMaster(unsigned long rate = 400000);
    
    //These write methods are blocking writes; they don't return until the
    //write succeeds or fails.
    //If stop is set to false, the line will be left hanging and a successive
    //read or write will trigger a repeated start. 
    uint8_t write(uint8_t address, uint8_t data, bool sendStop);
    uint8_t writen(uint8_t address, uint8_t *data, uint8_t n, bool sendStop);
    uint8_t write(uint8_t address, uint8_t reg, uint8_t data, bool sendStop);
    uint8_t writen(uint8_t address, uint8_t reg, uint8_t *data, uint8_t n, bool sendStop);
    
    uint8_t read(uint8_t address, uint8_t &data, bool sendStop);
    uint8_t readn(uint8_t address, uint8_t *data, uint8_t n, bool sendStop);
    uint8_t read(uint8_t address, uint8_t reg, uint8_t &data, bool sendStop);
    uint8_t readn(uint8_t address, uint8_t reg, uint8_t *data, uint8_t n, bool sendStop);
    
    //These methods return immediately after setting up the write operation with
    // a return value of I2C_CALLBACK or a different value in the event that
    // setting up the operation failed. These methods enable the interrupt flag
    // to use the TWI vector
    uint8_t iwritecb(uint8_t address, uint8_t data, i2c_complete cb);
    uint8_t iwritecb(uint8_t address, uint8_t *data, uint8_t n, i2c_complete cb);
    uint8_t iwritecb(uint8_t address, uint8_t reg, uint8_t data, i2c_complete cb);
    uint8_t iwritecb(uint8_t address, uint8_t reg, uint8_t *data, uint8_t n, i2c_complete cb);
    
    uint8_t ireadcb(uint8_t address, uint8_t &data, i2c_complete cb);
    uint8_t ireadcb(uint8_t address, uint8_t *data, uint8_t n, i2c_complete cb);
    uint8_t ireadcb(uint8_t address, uint8_t reg, uint8_t &data, i2c_complete cb);
    uint8_t ireadcb(uint8_t address, uint8_t reg, uint8_t *data, uint8_t n, i2c_complete cb);
    
    
    void stop();
    
    //should only be called from interrupt vector
    void _interrupt(uint8_t _twsr);
    
private:
    void start();
    void next(bool ack = false);
    uint8_t wait();
    
    //locally stores the data buffer for multi byte reads/writes
    uint8_t *_data;
    
    //stores the data count
    uint8_t _data_n;
    
    //sets whether using register addressing mode
    bool _register;
    
    //the register address to write or read from
    uint8_t _reg_addr;
    
    //flag on whether to send the stop condition at the end of the operation
    //or leave the line open for a repeated start
    bool _sendStop;
    
    //set when invoking a complete callback to protect against invalid states
    //and to know when to trigger repeated starts
    bool _inCallback;
    
    //stores the callback to invoke when using interrupt vector driven comms
    i2c_complete _callback;
    
    //sets the value for the interrupt enable bit to be used for bus control
    bool _twie;
    
    //stores internal state for responding to TWI events
    uint8_t _state;
}; //class I2C

//similar to Arduino's Serial, predefines an existing instance to be used
extern I2C i2c;

/*
//define the function pointer types for interrupt call

void initI2C(unsigned long rate = 400000);

//writes a single byte to a slave at address
//if callback is omitted, the function will block until all bytes have been
//processed or an error occurs
uint8_t i2c_write(uint8_t address, uint8_t data, i2c_complete_callback callback = 0);

//writes n bytes to a slave at address
//if callback is omitted, the function will block until all bytes have been
//processed or an error occurs
uint8_t i2c_write(uint8_t address, uint8_t *buf, int n, i2c_complete_callback callback = 0);

uint8_t i2c_write_register(uint8_t address, uint8_t reg_addr, uint8_t val, i2c_complete_callback callback = 0);

//reads a single byte from slave at address
//if callback is omitted, the function will block until all bytes have been
//processed or an error occurs
uint8_t i2c_read(uint8_t address, uint8_t *data, i2c_complete_callback callback = 0);

//reads n bytes from slave at address
//if callback is omitted, the function will block until all bytes have been
//processed or an error occurs
uint8_t i2c_read(uint8_t address, uint8_t *buf, int n, i2c_complete_callback callback = 0);
*/
#endif //I2C_H_
