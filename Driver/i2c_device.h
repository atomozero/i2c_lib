#ifndef I2C_DEVICE_H
#define I2C_DEVICE_H

#include <OS.h>
#include <drivers/PCI.h>
//#include <drivers/Drivers.h>
//#include <drivers/module.h>

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

// Struttura per i trasferimenti I2C
typedef struct {
    const uint8* send_buffer;
    size_t send_length;
    uint8* recv_buffer;
    size_t recv_length;
} i2c_transfer;

// Prototipi delle funzioni
status_t probe_i2c_devices();
void free_i2c_devices();
i2c_device_info* find_i2c_device(const char* name);

status_t i2c_device_init(i2c_device_info* device, uint8 slave_address);
status_t i2c_device_read(i2c_device_info* device, uint8* buffer, size_t length);
status_t i2c_device_write(i2c_device_info* device, const uint8* buffer, size_t length);
status_t i2c_device_read_register(i2c_device_info* device, uint8 reg, uint8* value);
status_t i2c_device_write_register(i2c_device_info* device, uint8 reg, uint8 value);
status_t i2c_device_transfer(i2c_device_info* device, i2c_transfer* transfers, size_t count);

// Definizioni per i comandi IOCTL generici dei dispositivi I2C
enum {
    I2C_DEVICE_IOCTL_GET_FUNCTIONALITY = B_DEVICE_OP_CODES_END + 500,
    I2C_DEVICE_IOCTL_SET_RETRIES,
    I2C_DEVICE_IOCTL_SET_TIMEOUT,
    I2C_DEVICE_IOCTL_SET_SLAVE_ADDR,
    // Aggiungi altri codici IOCTL secondo necessità
};



// Definizione delle funzionalità I2C
#define I2C_FUNC_I2C                    0x00000001
#define I2C_FUNC_10BIT_ADDR             0x00000002
#define I2C_FUNC_PROTOCOL_MANGLING      0x00000004
#define I2C_FUNC_SMBUS_PEC              0x00000008
#define I2C_FUNC_NOSTART                0x00000010
#define I2C_FUNC_SMBUS_BLOCK_PROC_CALL  0x00000020
#define I2C_FUNC_SMBUS_QUICK            0x00010000
#define I2C_FUNC_SMBUS_READ_BYTE        0x00020000
#define I2C_FUNC_SMBUS_WRITE_BYTE       0x00040000
#define I2C_FUNC_SMBUS_READ_BYTE_DATA   0x00080000
#define I2C_FUNC_SMBUS_WRITE_BYTE_DATA  0x00100000
#define I2C_FUNC_SMBUS_READ_WORD_DATA   0x00200000
#define I2C_FUNC_SMBUS_WRITE_WORD_DATA  0x00400000
#define I2C_FUNC_SMBUS_PROC_CALL        0x00800000
#define I2C_FUNC_SMBUS_READ_BLOCK_DATA  0x01000000
#define I2C_FUNC_SMBUS_WRITE_BLOCK_DATA 0x02000000
#define I2C_FUNC_SMBUS_READ_I2C_BLOCK   0x04000000
#define I2C_FUNC_SMBUS_WRITE_I2C_BLOCK  0x08000000

#endif // I2C_DEVICE_H
