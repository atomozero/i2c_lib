#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <drivers/Drivers.h>
#include <drivers/module.h>
 

// Struttura per le informazioni del dispositivo I2C
typedef struct i2c_device_info {
    uint32 base_addr;
    area_id register_area;
    void* mapped_registers;
    uint8 irq;
    uint16 vendor_id;
    uint16 device_id;
    uint8 slave_addr;
    
    // Funzioni per le operazioni del dispositivo
    status_t (*read)(struct i2c_device_info* device, off_t position, void* buffer, size_t* numBytes);
    status_t (*write)(struct i2c_device_info* device, off_t position, const void* buffer, size_t* numBytes);
    status_t (*control)(struct i2c_device_info* device, uint32 op, void* arg, size_t len);
} i2c_device_info;

// Dichiarazioni delle funzioni del driver con firme corrette
float init_hardware(device_node* node);
status_t init_driver(device_node* node);
void uninit_driver(device_node* node, void* cookie);
bool publish_devices(device_node* parent);
device_hooks* find_device(const char* name);

// Dichiarazioni delle funzioni del dispositivo
status_t i2c_touchpad_open(const char* name, uint32 flags, void** cookie);
status_t i2c_touchpad_close(void* cookie);
status_t i2c_touchpad_free(void* cookie);
status_t i2c_touchpad_control(void* cookie, uint32 op, void* arg, size_t len);
status_t i2c_touchpad_read(void* cookie, off_t position, void* buffer, size_t* numBytes);
status_t i2c_touchpad_write(void* cookie, off_t position, const void* buffer, size_t* numBytes);

// Dichiarazione corretta della struttura module_info
extern module_info gI2CTouchpadDriverModule; 

// Dichiarazioni delle funzioni di supporto
status_t probe_i2c_devices();
void free_i2c_devices();
i2c_device_info* find_i2c_device(const char* name);

// Definizioni per i comandi IOCTL specifici del touchpad
enum {
    I2C_TOUCHPAD_IOCTL_GET_INFO = B_DEVICE_OP_CODES_END + 1000,
    I2C_TOUCHPAD_IOCTL_SET_PARAMETERS,
    // Aggiungi altri codici IOCTL secondo necessità
};

#endif // I2C_DRIVER_H
