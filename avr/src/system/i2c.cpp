#include <system/i2c.h>

//from AVR Libc, TWI interface status defines
#include <util/twi.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#define STATE_IDLE 0x00
#define STATE_MT   0x01
#define STATE_MR   0x02
#define STATE_ST   0x03
#define STATE_SR   0x04

//this array buffers single byte writes with a second byte for a register
uint8_t __buf[2];

//The predefined controller interface
I2C i2c;

ISR(TWI_vect) {
    i2c._interrupt(TW_STATUS);
}

void I2C::start() {
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)|((_twie ? 1 : 0)<<TWIE);
}

void I2C::stop() {
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN)|((_twie ? 1 : 0)<<TWIE);
}

void I2C::next(bool ack) {    
    TWCR = (1<<TWINT)|(1<<TWEN)|((ack ? 1 : 0)<<TWEA)|((_twie ? 1 : 0)<<TWIE);
}

uint8_t I2C::wait() {
    while ((TWCR & (1<<TWINT)) == 0);
    return TW_STATUS;
}

I2C::I2C() {
    _data = 0;
    _data_n = 0;
    _register = false;
    _reg_addr = 0;
    _sendStop = true;
    _inCallback = false;
    _callback = 0;
    _twie = false;
    _state = STATE_IDLE;
}

void I2C::initMaster(unsigned long rate) {
    //rate = F_CPU / (16 + 2(TWBR) * Prescale)
    //scaler = (F_CPU / (2 * Rate)) - 8 assuming prescale of 1
    
    //automatically determine prescale value to be as small as possible while
    //fitting the bit rate divider into 8 bits
    int bitRate = F_CPU / (2 * rate) - 8;
    
    uint8_t prescale = 0;
    
    while (bitRate > 255) {
        prescale++;
        bitRate /= 4;
        
        if (prescale > 3) {
            //error case, can't handle that large of a rate
            //for now do nothing
        }
    }
    
    TWSR = prescale & 0x3;
    TWBR = (uint8_t)bitRate;
    
    TWCR = (1<<TWEN); // enable the TWI interface
}

uint8_t I2C::writen(uint8_t address, uint8_t *data, uint8_t n, bool sendStop) {
    //handle a few error cases to prevent invalid bus states or deadlocks
    if (_inCallback) {
        return I2C_NOBLOCK;
    } else if (_twie) {
        return I2C_BUSY;
    }
    
    //send start condition on TWI controller
    //controller waits for bus to be free
    start();
    
    //success is TW_START
    switch (wait()) {
        case TW_BUS_ERROR:
            if (sendStop) stop();
            return I2C_BUSERROR;
    }
    
    //set slave address and write flag
    TWDR = (address<<1) | TW_WRITE;
    
    //tells the controller to actually write the address to the bus
    next();
    
    //success is TW_MT_SLA_ACK
    switch (wait()) {
        case TW_BUS_ERROR:
            if (sendStop) stop();
            return I2C_BUSERROR;
        
        case TW_MT_SLA_NACK:
            if (sendStop) stop();
            return I2C_ADDRNACK;
    }
    
    while (n) {
        if (_register) {
            _register = false;
            TWDR = _reg_addr;           
        } else {
            TWDR = *data;           
            n--;
            if (n) data++;
        }
        
        next();
        
        //success is TW_MT_DATA_ACK
        switch(wait()) {
            case TW_BUS_ERROR:
                if (sendStop) stop();
                return I2C_BUSERROR;
                
            case TW_MT_DATA_NACK:
                if (sendStop) stop();
                return I2C_DATANACK;
        }
    }
    
    if (sendStop) stop();
    
    return I2C_SUCCESS;
}

uint8_t I2C::write(uint8_t address, uint8_t data, bool sendStop) {
    __buf[0] = data;
    return writen(address, __buf, 1, sendStop);
}

uint8_t I2C::write(uint8_t address, uint8_t reg, uint8_t data, bool sendStop) {
    _reg_addr = reg;
    _register = true;
    __buf[0] = data;
    return writen(address, __buf, 1, sendStop);
}

uint8_t I2C::writen(uint8_t address, uint8_t reg, uint8_t *data, uint8_t n, bool sendStop) {
    _reg_addr = reg;
    _register = true;
    return writen(address, data, n, sendStop);
}

uint8_t I2C::readn(uint8_t address, uint8_t *data, uint8_t n, bool sendStop) {
    //handle a few error cases to prevent invalid bus states or deadlocks
    if (_inCallback) {
        return I2C_NOBLOCK;
    } else if (_twie) {
        return I2C_BUSY;
    }
    
    start();
    
    //success is TW_START
    switch (wait()) {
        case TW_BUS_ERROR:
            if (sendStop) stop();
            return I2C_BUSERROR;
    }
    
    //set slave address and read flag
    TWDR = (address<<1) | TW_READ;
    
    //tells the controller to actually write the address to the bus
    next();
    
    //success is TW_MR_SLA_ACK
    switch (wait()) {
        case TW_BUS_ERROR:
            if (sendStop) stop();
            return I2C_BUSERROR;
            
        case TW_MR_SLA_NACK:
            if (sendStop) stop();
            return I2C_BUSERROR;
    }
    
    //addressed slave responded, just carry on with ack as well
    next(true);
    
    while (n) {        
        switch (wait()) {
            case TW_BUS_ERROR:
                if (sendStop) stop();
                return I2C_BUSERROR;
            
            case TW_MR_DATA_NACK:
                if (sendStop) stop();
                return I2C_DATANACK;
        }
        
        *data = TWDR;
        n--;
        if (n) {
            data++;
        }
        
        next(n > 0);
    }
    
    //success is TW_MR_DATA_NACK
    switch(wait()) {
        case TW_BUS_ERROR:
            if (sendStop) stop();
            return I2C_BUSERROR;
    }
    
    if (sendStop) stop();
    return I2C_SUCCESS;
}

uint8_t I2C::read(uint8_t address, uint8_t &data, bool sendStop) {
    return readn(address, &data, 1, sendStop);
}

uint8_t I2C::read(uint8_t address, uint8_t reg, uint8_t &data, bool sendStop) {
    //first use master transmit mode to send register to read from
    uint8_t status = write(address, reg, false);
    
    if (status == I2C_SUCCESS) {
        status = readn(address, &data, 1, sendStop);
    }
    
    return status;
}

uint8_t I2C::readn(uint8_t address, uint8_t reg, uint8_t *data, uint8_t n, bool sendStop) {
    //first use master transmit mode to send register to read from
    uint8_t status = write(address, reg, false);
    
    if (status == I2C_SUCCESS) {
        status = readn(address, data, n, sendStop);
    }
    
    return status;
}

uint8_t I2C::iwritecb(uint8_t address, uint8_t *data, uint8_t n, i2c_complete cb) {
    return I2C_UNKNOWN;
}

uint8_t I2C::iwritecb(uint8_t address, uint8_t data, i2c_complete cb) {
    return I2C_UNKNOWN;
}

uint8_t I2C::iwritecb(uint8_t address, uint8_t reg, uint8_t data, i2c_complete cb) {
    return I2C_UNKNOWN;
}

uint8_t I2C::iwritecb(uint8_t address, uint8_t reg, uint8_t *data, uint8_t n, i2c_complete cb) {
    return I2C_UNKNOWN;
}

uint8_t I2C::ireadcb(uint8_t address, uint8_t &data, i2c_complete cb) {
    return I2C_UNKNOWN;
}

uint8_t I2C::ireadcb(uint8_t address, uint8_t *data, uint8_t n, i2c_complete cb) {
    return I2C_UNKNOWN;
}

uint8_t I2C::ireadcb(uint8_t address, uint8_t reg, uint8_t &data, i2c_complete cb) {
    return I2C_UNKNOWN;
}

uint8_t I2C::ireadcb(uint8_t address, uint8_t reg, uint8_t *data, uint8_t n, i2c_complete cb) {
    return I2C_UNKNOWN;
}

void I2C::_interrupt(uint8_t _twsr) {
}

/*
//internal defines used by this library, some of these will need to move
//to header file for status responses

#define I2C_IDLE    0x00
#define I2C_MT      0x01
#define I2C_MR      0x02
#define I2C_ST      0x03
#define I2C_SR      0x04

#define I2C_START TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)|(1<<TWIE)
#define I2C_STOP TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN)|(1<<TWIE)

#define I2C_CONTINUE TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)

#define I2C_ACK TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA)|(1<<TWIE)
#define I2C_NACK I2C_CONTINUE

//used to store internal state to know what to do next when the TWI
//interrupt fires
volatile uint8_t _mode = I2C_IDLE;

//used to signify a repeated start should be used
//in the event of typical behavior like addressing a slave with a register
//and reading back that value. Enables calling a write/read function from a
//callback function
volatile bool _repeated = false;

//flag while in a callback so that calls to read/write from within a callback
//trigger a repeated start
volatile bool _callback = false;

volatile uint8_t *_data_ptr = 0;
volatile int _data_n = 0;

uint8_t _write1 = 0;

volatile i2c_complete_callback _complete_callback;
volatile uint8_t _status;

volatile uint8_t _address;

volatile bool _register = false;
volatile uint8_t _reg_addr;

void complete(uint8_t status);

ISR(TWI_vect) {    
    switch (TW_STATUS) {
        case TW_START:
        case TW_REP_START:
            switch (_mode) {
                case I2C_MT:
                    TWDR = (_address<<1) | TW_WRITE;
                    I2C_CONTINUE;
                    break;
                    
                case I2C_MR:
                    TWDR = (_address<<1) | TW_READ;
                    I2C_CONTINUE;
                    break;
            }
            break;
            
        case TW_MT_SLA_ACK:
            //slave has been addressed, send data byte
            if (_register) {
                _register = false;
                TWDR = _reg_addr;
            } else {
                TWDR = *_data_ptr;
                _data_n--;
                
                if (_data_n) _data_ptr++;
            }
            
            I2C_CONTINUE;
            break;
            
        case TW_MT_SLA_NACK:
            complete(I2C_ADDRNACK);
            break;
            
        case TW_MT_DATA_ACK:
            if (_data_n) {
                TWDR = *_data_ptr;
                _data_n--;
                
                if (_data_n) _data_ptr++;
                
                I2C_CONTINUE;
            } else {
                complete(I2C_SUCCESS);
            }
            break;

        case TW_MT_DATA_NACK:
            complete(I2C_DATANACK);
            break;

        case 0x38: //TW_MT_ARB_LOST or TW_MR_ARB_LOST
            I2C_START;
            break;

        case TW_MR_SLA_ACK:
            I2C_ACK;
            break;

        case TW_MR_SLA_NACK:
            complete(I2C_ADDRNACK);
            break;

        case TW_MR_DATA_ACK:
            if (_register) {
                _register = false;
                TWDR = _reg_addr;
                I2C_ACK;
            } else {
                *_data_ptr = TWDR;
                
                _data_n--;
                
                if (_data_n) {
                    _data_ptr++;
                    I2C_ACK;
                } else {
                    I2C_NACK;
                }
            }
            break;

        case TW_MR_DATA_NACK:
            if (_data_n)
                complete(I2C_DATANACK);
            else
                complete(I2C_SUCCESS);
            break;

        case TW_ST_SLA_ACK:
            break;

        case TW_ST_ARB_LOST_SLA_ACK:
            break;

        case TW_ST_DATA_ACK:
            break;

        case TW_ST_DATA_NACK:
            break;

        case TW_ST_LAST_DATA:
            break;

        case TW_SR_SLA_ACK:
            break;

        case TW_SR_ARB_LOST_SLA_ACK:
            break;

        case TW_SR_GCALL_ACK:
            break;

        case TW_SR_ARB_LOST_GCALL_ACK:
            break;

        case TW_SR_DATA_ACK:
            break;

        case TW_SR_DATA_NACK:
            break;

        case TW_SR_GCALL_DATA_ACK:
            break;

        case TW_SR_GCALL_DATA_NACK:
            break;
            
        case TW_SR_STOP:
            break;

        case TW_NO_INFO:
            break;

        case TW_BUS_ERROR:
            complete(I2C_BUSERROR);
            I2C_STOP;
            break;
    }
}

void complete(uint8_t status) {
    _mode = I2C_IDLE;
    
    if (_complete_callback) {
        (*_complete_callback)(status);
    } else {
        _status = status;
    }
    
    if (_repeated) {
        _repeated = false;
        I2C_START;
    } else {
        I2C_STOP;
    }
}

//sets up TWI interface
void initI2C(unsigned long rate) {
    //rate = F_CPU / (16 + 2(TWBR) * Prescale)
    
    //scaler = (F_CPU / (2 * Rate)) + 8 assumign prescale of 1
    
    //automatically determine prescale value to be as small as possible while
    //fitting the bit rate divider into 8 bits
    int bitRate = F_CPU / (2 * rate) - 8;
    
    uint8_t prescale = 0;
    
    while (bitRate > 255) {
        prescale++;
        bitRate /= 4;
        
        if (prescale > 3) {
            //error case, can't handle that large of a rate
            //for now do nothing
        }
    }
    
    TWSR = prescale & 0x3;
    TWBR = (uint8_t)bitRate;
    
    TWCR = (1<<TWEN) | (1<<TWIE); // enable the TWI interface
}

uint8_t i2c_write(uint8_t address, uint8_t *buf, int n, i2c_complete_callback callback) {
    _mode = I2C_MT;
    _data_n = n;
    _data_ptr = &_write1;
    _address = address;
    _complete_callback = callback;
    
    if (_callback) { //nested call from a callback?
        if (!callback)
            return I2C_NOBLOCK; //can't call blocking mode from callback
        
        _repeated = true;
    } else {
        I2C_START;
    }
    
    if (callback)
        return I2C_ASYNC; //status returned that the callback is being used
    
    //no callback, wait for operation to complete
    while (_mode != I2C_IDLE);
    
    return _status;
}

uint8_t i2c_write(uint8_t address, uint8_t data, i2c_complete_callback callback) {
    _write1 = data;
    
    return i2c_write(address, &_write1, 1, callback);
}

uint8_t i2c_write_register(uint8_t address, uint8_t reg_addr, uint8_t val, i2c_complete_callback callback) {
    _write1 = val;
    _reg_addr = reg_addr;
    _register = true;
    
    return i2c_write(address, &_write1, 1, callback);
}

uint8_t i2c_read(uint8_t address, uint8_t *buf, int n, i2c_complete_callback callback) {
    _address = address;
    _data_ptr = buf;
    _data_n = n;
    _mode = I2C_MR;
    _complete_callback = callback;
    
    if (_callback) {
        if (!callback)
            return I2C_NOBLOCK; //can't call blocking mode from callback
            
        _repeated = true;
    } else {
        I2C_START;
    }
    
    if (callback)
        return I2C_ASYNC;
    
    while (_mode != I2C_IDLE);
    
    return _status;
}

uint8_t i2c_read(uint8_t address, uint8_t *data, i2c_complete_callback callback) {
    return i2c_read(address, data, 1, callback);
}
*/
