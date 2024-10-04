#ifndef I2C_UTIL_H
#define I2C_UTIL_H

#include <OS.h>
#include <drivers/KernelExport.h>

// Definizioni delle costanti I2C
#define I2C_SLAVE 0x0703
#define I2C_FUNC_I2C 0x00000001
#define I2C_FUNC_SMBUS_EMUL 0x00000002

// Vendor e Device ID
#define INTEL_VENDOR_ID 0x8086
#define INTEL_TIGER_LAKE_I2C_CONTROLLER_0 0xa0e8
#define INTEL_TIGER_LAKE_I2C_CONTROLLER_1 0xa0e9

// Registri del controller I2C (esempio, potrebbero variare a seconda dell'hardware specifico)
#define I2C_CON         0x00 // Control Register
#define I2C_TAR         0x04 // Target Address Register
#define I2C_SAR         0x08 // Slave Address Register
#define I2C_HS_MADDR    0x0C // High Speed Master Mode Code Address Register
#define I2C_DATA_CMD    0x10 // Data Command Register
#define I2C_SS_SCL_HCNT 0x14 // Standard Speed I2C Clock SCL High Count Register
#define I2C_SS_SCL_LCNT 0x18 // Standard Speed I2C Clock SCL Low Count Register
#define I2C_FS_SCL_HCNT 0x1C // Fast Speed I2C Clock SCL High Count Register
#define I2C_FS_SCL_LCNT 0x20 // Fast Speed I2C Clock SCL Low Count Register
#define I2C_INTR_STAT   0x2C // Interrupt Status Register
#define I2C_INTR_MASK   0x30 // Interrupt Mask Register
#define I2C_RAW_INTR_STAT 0x34 // Raw Interrupt Status Register
#define I2C_RX_TL       0x38 // Receive FIFO Threshold Register
#define I2C_TX_TL       0x3C // Transmit FIFO Threshold Register
#define I2C_CLR_INTR    0x40 // Clear Combined and Individual Interrupt Register
#define I2C_CLR_RX_UNDER 0x44 // Clear RX_UNDER Interrupt Register
#define I2C_CLR_RX_OVER 0x48 // Clear RX_OVER Interrupt Register
#define I2C_CLR_TX_OVER 0x4C // Clear TX_OVER Interrupt Register
#define I2C_CLR_RD_REQ  0x50 // Clear RD_REQ Interrupt Register
#define I2C_CLR_TX_ABRT 0x54 // Clear TX_ABRT Interrupt Register
#define I2C_CLR_RX_DONE 0x58 // Clear RX_DONE Interrupt Register
#define I2C_CLR_ACTIVITY 0x5C // Clear ACTIVITY Interrupt Register
#define I2C_CLR_STOP_DET 0x60 // Clear STOP_DET Interrupt Register
#define I2C_CLR_START_DET 0x64 // Clear START_DET Interrupt Register
#define I2C_CLR_GEN_CALL 0x68 // Clear GEN_CALL Interrupt Register
#define I2C_ENABLE      0x6C // Enable Register
#define I2C_STATUS      0x70 // Status Register
#define I2C_TXFLR       0x74 // Transmit FIFO Level Register
#define I2C_RXFLR       0x78 // Receive FIFO Level Register
#define I2C_SDA_HOLD    0x7C // SDA Hold Time Length Register
#define I2C_TX_ABRT_SOURCE 0x80 // Transmit Abort Source Register

// Macro per accesso ai registri
#define READ_REG32(base, reg) (*((volatile uint32 *)((base) + (reg))))
#define WRITE_REG32(base, reg, value) (*((volatile uint32 *)((base) + (reg))) = (value))

// Funzioni di utilità
static inline void i2c_set_bit(void* base, uint32 reg, uint32 bit) {
    uint32 value = READ_REG32(base, reg);
    WRITE_REG32(base, reg, value | (1 << bit));
}

static inline void i2c_clear_bit(void* base, uint32 reg, uint32 bit) {
    uint32 value = READ_REG32(base, reg);
    WRITE_REG32(base, reg, value & ~(1 << bit));
}

static inline bool i2c_is_bit_set(void* base, uint32 reg, uint32 bit) {
    return (READ_REG32(base, reg) & (1 << bit)) != 0;
}

// Funzioni di debug
#if DEBUG
    #define I2C_DEBUG_PRINT(x...) dprintf(DRIVER_NAME ": " x)
#else
    #define I2C_DEBUG_PRINT(x...)
#endif

// Funzione di utilità per il calcolo del timeout
static inline bigtime_t calculate_timeout(bigtime_t timeout) {
    return system_time() + timeout;
}

// Funzione di utilità per verificare il timeout
static inline bool is_timeout(bigtime_t timeout) {
    return system_time() > timeout;
}

#endif // I2C_UTIL_H
