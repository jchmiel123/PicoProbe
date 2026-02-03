// I2C Sniffer - Passive I2C bus analyzer
// Uses PIO for precise timing capture

#ifndef I2C_SNIFFER_H
#define I2C_SNIFFER_H

#include <Arduino.h>

// Buffer for captured transactions
#define TX_BUFFER_SIZE 256
#define MAX_BYTES_PER_TX 64

struct I2CTransaction {
    uint32_t timestamp;
    uint8_t address;
    bool is_read;
    uint8_t data[MAX_BYTES_PER_TX];
    uint8_t length;
    bool nack;
};

class I2CSniffer {
public:
    I2CSniffer(int sda_pin, int scl_pin);

    void begin();
    void process();
    void reset();
    void printStats();
    void toggleHex();
    void togglePause();

private:
    int _sda_pin;
    int _scl_pin;

    // State machine
    enum State {
        IDLE,
        START,
        ADDRESS,
        DATA,
        ACK
    };

    volatile State _state;
    volatile uint8_t _bit_count;
    volatile uint8_t _current_byte;
    volatile bool _in_transaction;

    // Current transaction being captured
    I2CTransaction _current_tx;

    // Circular buffer for completed transactions
    I2CTransaction _tx_buffer[TX_BUFFER_SIZE];
    volatile int _tx_head;
    volatile int _tx_tail;

    // Stats
    uint32_t _tx_count;
    uint32_t _error_count;
    uint32_t _start_time;

    // Options
    bool _hex_output;
    bool _paused;

    // ISR handlers
    static I2CSniffer* _instance;
    static void sclRisingISR();
    static void sdaChangeISR();

    void onSclRising();
    void onSdaChange();
    void completeByte();
    void startTransaction();
    void endTransaction();
    void printTransaction(I2CTransaction& tx);
};

#endif
