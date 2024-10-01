#ifndef I2C_H
#define I2C_H
#define I2C_SET_SLAVE 0x0703

#include <OS.h>
#include <stdint.h>

class I2CBus {
public:
    // Costruttore e distruttore
    I2CBus(int bus_number);
    ~I2CBus();

    // Inizializzazione e de-inizializzazione del bus I2C
    status_t init();
    status_t deinit();

    // Operazioni di lettura e scrittura
    status_t write(uint8 address, const uint8* data, size_t length);
    status_t read(uint8 address, uint8* buffer, size_t length);

    // Imposta la velocità del bus I2C
    status_t set_speed(uint32 speed);

    // Funzioni avanzate per la gestione dei registri I2C
    status_t write_register(uint8 address, uint8 reg, uint8 data);
    status_t read_register(uint8 address, uint8 reg, uint8* data);

private:
    int f_bus_number;
    bool f_initialized;
    uint32 f_speed;
};

#endif  // I2C_H
