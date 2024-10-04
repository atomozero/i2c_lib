#ifndef I2C_CONTROLLER_H
#define I2C_CONTROLLER_H

#include <OS.h>
#include "i2c_driver.h"

// Costanti per i controller I2C Intel
#define INTEL_VENDOR_ID 0x8086
#define TIGER_LAKE_I2C_CONTROLLER_0 0xa0e8
#define TIGER_LAKE_I2C_CONTROLLER_1 0xa0e9

// Prototipi delle funzioni
status_t probe_i2c_devices();
status_t init_i2c_controller(i2c_device_info* device);
status_t i2c_transfer(i2c_device_info* device, int addr, const uint8* write_buf, size_t write_len, uint8* read_buf, size_t read_len);
void free_i2c_devices();
i2c_device_info* find_i2c_device(const char* name);

// Funzioni di utilità per l'accesso ai registri
static inline uint32 read32(const void* address) {
    return *(volatile uint32*)address;
}

static inline void write32(void* address, uint32 value) {
    *(volatile uint32*)address = value;
}

// Struttura per la configurazione del controller I2C
typedef struct {
    uint32 speed;         // Velocità del bus in Hz
    uint8 addressing_mode; // 7-bit o 10-bit
    uint8 duty_cycle;     // Per la modalità Fast Plus (opzionale)
} i2c_controller_config;

// Funzioni aggiuntive per la gestione del controller
status_t i2c_controller_set_config(i2c_device_info* device, i2c_controller_config* config);
status_t i2c_controller_get_config(i2c_device_info* device, i2c_controller_config* config);

// Funzioni per operazioni I2C di alto livello
status_t i2c_read_register(i2c_device_info* device, uint8 slave_addr, uint8 reg_addr, uint8* data, size_t length);
status_t i2c_write_register(i2c_device_info* device, uint8 slave_addr, uint8 reg_addr, const uint8* data, size_t length);

// Definizioni per la gestione delle interruzioni
#define I2C_INTR_DEFAULT_MASK 0x24  // Esempio: abilita interruzioni per completamento TX e RX

status_t i2c_controller_setup_interrupt(i2c_device_info* device);
int32 i2c_interrupt_handler(void* data);

#endif // I2C_CONTROLLER_H
