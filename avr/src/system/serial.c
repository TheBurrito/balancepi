#include <system/serial.h>
#include <util/circularbuffer.h>
#include <avr/io.h>
#include <avr/interrupt.h>

static int uart_putchar(char c, FILE *stream);
static int uart_getchar(FILE *stream);

CircularBuffer _rx = CREATE_BUFFER();
CircularBuffer _tx = CREATE_BUFFER();

ISR(USART_RX_vect) {
    cb_put(&_rx, UDR0);
}

ISR(USART_UDRE_vect) {
    if (cb_isEmpty(&_tx)) {
        UCSR0B &= ~(1<<UDRIE0);
    } else {
        UDR0 = cb_get(&_tx);
    }
}

static FILE _uartStream = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

void initSerial(long baud) {
    stdin = stdout = &_uartStream;
    
    uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;
    UCSR0A = 1 << U2X0;

    // hardcoded exception for 57600 for compatibility with the bootloader
    // shipped with the Duemilanove and previous boards and the firmware
    // on the 8U2 on the Uno and Mega 2560. Also, The baud_setting cannot
    // be > 4095, so switch back to non-u2x mode if the baud rate is too
    // low.
    if (((F_CPU == 16000000UL) && (baud == 57600)) || (baud_setting >4095)) {
        UCSR0A = 0;
        baud_setting = (F_CPU / 8 / baud - 1) / 2;
    }

    // assign the baud_setting, a.k.a. ubrr (USART Baud Rate Register)
    UBRR0H = baud_setting >> 8;
    UBRR0L = baud_setting;
    
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

    UCSR0B |= (1<<RXEN0);
    UCSR0B |= (1<<TXEN0);
    UCSR0B |= (1<<RXCIE0);
    UCSR0B &= ~(1<<UDRIE0);
}

static int uart_putchar(char c, FILE *stream) {
    if (c == '\n')
        uart_putchar('\r', stream);
    
    while (cb_isFull(&_tx));
    
    cb_put(&_tx, c);
    
    UCSR0B |= (1<<UDRIE0);
    
    return 0;
}

static int uart_getchar(FILE *stream) {
    while (cb_isEmpty(&_rx));
    
    return cb_get(&_rx);
}
