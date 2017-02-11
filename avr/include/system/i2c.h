#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

#define I2C_UNKNOWN     0x00
#define I2C_SUCCESS     0x01
#define I2C_BUSERROR    0x02
#define I2C_ASYNC       0x03
#define I2C_ADDRNACK    0x04
#define I2C_DATANACK    0x05
#define I2C_BUSY        0x06
#define I2C_NOBLOCK     0x07

//define the function pointer types for interrupt callbacks
typedef void (*i2c_complete_callback)(uint8_t status);

void initI2C(unsigned long rate = 400000);

//writes a single byte to a slave at address
//if callback is omitted, the function will block until all bytes have been
//processed or an error occurs
uint8_t i2c_write(uint8_t address, uint8_t data, i2c_complete_callback callback = 0);

//writes n bytes to a slave at address
//if callback is omitted, the function will block until all bytes have been
//processed or an error occurs
uint8_t i2c_write(uint8_t address, uint8_t *buf, int n, i2c_complete_callback callback = 0);

//reads a single byte from slave at address
//if callback is omitted, the function will block until all bytes have been
//processed or an error occurs
uint8_t i2c_read(uint8_t address, uint8_t *data, i2c_complete_callback callback = 0);

//reads n bytes from slave at address
//if callback is omitted, the function will block until all bytes have been
//processed or an error occurs
uint8_t i2c_read(uint8_t address, uint8_t *buf, int n, i2c_complete_callback callback = 0);

#endif //I2C_H_
