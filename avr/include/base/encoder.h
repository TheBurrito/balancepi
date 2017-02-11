#ifndef ENCODER_H_
#define ENCODER_H_

#include <stdint.h>

class Encoder {
public:
    Encoder(uint8_t interrupt, volatile uint8_t *bPort, uint8_t bPin);
    ~Encoder();
    
    int32_t get();
    int32_t getAndClear();
    
private:
    uint8_t _int;
};

extern Encoder encA;
extern Encoder encB;

#endif //ENCODER_H_
