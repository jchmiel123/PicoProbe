// I2C Sniffer Implementation
// Passive sniffing using GPIO interrupts

#include "i2c_sniffer.h"

I2CSniffer* I2CSniffer::_instance = nullptr;

I2CSniffer::I2CSniffer(int sda_pin, int scl_pin)
    : _sda_pin(sda_pin), _scl_pin(scl_pin) {
    _instance = this;
    _state = IDLE;
    _bit_count = 0;
    _current_byte = 0;
    _in_transaction = false;
    _tx_head = 0;
    _tx_tail = 0;
    _tx_count = 0;
    _error_count = 0;
    _hex_output = true;
    _paused = false;
}

void I2CSniffer::begin() {
    // Configure pins as inputs with pullups disabled
    // (external pullups should be on the I2C bus)
    pinMode(_sda_pin, INPUT);
    pinMode(_scl_pin, INPUT);

    // Attach interrupts
    attachInterrupt(digitalPinToInterrupt(_scl_pin), sclRisingISR, RISING);
    attachInterrupt(digitalPinToInterrupt(_sda_pin), sdaChangeISR, CHANGE);

    _start_time = millis();
}

void I2CSniffer::sclRisingISR() {
    if (_instance) _instance->onSclRising();
}

void I2CSniffer::sdaChangeISR() {
    if (_instance) _instance->onSdaChange();
}

void I2CSniffer::onSclRising() {
    if (_paused) return;

    // Sample SDA on rising edge of SCL
    int sda = digitalRead(_sda_pin);

    if (_state == IDLE) return;

    if (_state == ACK) {
        // This is the ACK/NACK bit
        if (_bit_count == 0) {
            // First byte is address
            _current_tx.address = _current_byte >> 1;
            _current_tx.is_read = _current_byte & 0x01;
        } else {
            // Data byte
            if (_current_tx.length < MAX_BYTES_PER_TX) {
                _current_tx.data[_current_tx.length++] = _current_byte;
            }
        }

        _current_tx.nack = (sda == HIGH);
        _state = DATA;
        _bit_count = 0;
        _current_byte = 0;
        return;
    }

    // Shift in data bit
    _current_byte = (_current_byte << 1) | sda;
    _bit_count++;

    if (_bit_count == 8) {
        _state = ACK;
    }
}

void I2CSniffer::onSdaChange() {
    if (_paused) return;

    int scl = digitalRead(_scl_pin);
    int sda = digitalRead(_sda_pin);

    // START condition: SDA falls while SCL is high
    // STOP condition: SDA rises while SCL is high

    if (scl == HIGH) {
        if (sda == LOW) {
            // START condition
            startTransaction();
        } else {
            // STOP condition
            endTransaction();
        }
    }
}

void I2CSniffer::startTransaction() {
    _state = DATA;
    _bit_count = 0;
    _current_byte = 0;
    _in_transaction = true;

    memset(&_current_tx, 0, sizeof(_current_tx));
    _current_tx.timestamp = millis();
}

void I2CSniffer::endTransaction() {
    if (!_in_transaction) return;

    _in_transaction = false;
    _state = IDLE;

    // Add to buffer
    int next_head = (_tx_head + 1) % TX_BUFFER_SIZE;
    if (next_head != _tx_tail) {
        _tx_buffer[_tx_head] = _current_tx;
        _tx_head = next_head;
        _tx_count++;
    } else {
        _error_count++; // Buffer overflow
    }
}

void I2CSniffer::process() {
    // Print any completed transactions
    while (_tx_tail != _tx_head) {
        printTransaction(_tx_buffer[_tx_tail]);
        _tx_tail = (_tx_tail + 1) % TX_BUFFER_SIZE;
    }
}

void I2CSniffer::printTransaction(I2CTransaction& tx) {
    // Format: [timestamp] ADDR W/R: DATA...
    Serial.printf("[%8lu] ", tx.timestamp - _start_time);

    if (_hex_output) {
        Serial.printf("0x%02X ", tx.address);
    } else {
        Serial.printf("%3d ", tx.address);
    }

    Serial.print(tx.is_read ? "R: " : "W: ");

    for (int i = 0; i < tx.length; i++) {
        if (_hex_output) {
            Serial.printf("%02X ", tx.data[i]);
        } else {
            Serial.printf("%3d ", tx.data[i]);
        }
    }

    if (tx.nack) {
        Serial.print("[NACK]");
    }

    Serial.println();
}

void I2CSniffer::reset() {
    _tx_head = 0;
    _tx_tail = 0;
    _tx_count = 0;
    _error_count = 0;
    _start_time = millis();
}

void I2CSniffer::printStats() {
    uint32_t elapsed = (millis() - _start_time) / 1000;
    Serial.println("\n--- Statistics ---");
    Serial.printf("Transactions: %lu\n", _tx_count);
    Serial.printf("Errors/overflows: %lu\n", _error_count);
    Serial.printf("Runtime: %lu seconds\n", elapsed);
    Serial.printf("Avg rate: %.1f tx/sec\n", elapsed > 0 ? (float)_tx_count / elapsed : 0);
    Serial.println("------------------\n");
}

void I2CSniffer::toggleHex() {
    _hex_output = !_hex_output;
    Serial.printf("Output mode: %s\n", _hex_output ? "HEX" : "DECIMAL");
}

void I2CSniffer::togglePause() {
    _paused = !_paused;
    Serial.printf("Capture: %s\n", _paused ? "PAUSED" : "RUNNING");
}
