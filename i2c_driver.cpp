#include <Drivers.h>
#include <KernelExport.h>
#include <PCI.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define DRIVER_NAME "i2c_tigerlake"
#define DEVICE_NAME "bus/i2c/tigerlake/%d"

#define INTEL_VENDOR_ID 0x8086
#define INTEL_TIGERLAKE_I2C_DEVICE_ID 0xa0e9

// Registri I2C (da verificare con la documentazione Intel)
#define I2C_CON         0x00  // Control Register
#define I2C_TAR         0x04  // Target Address Register
#define I2C_SAR         0x08  // Slave Address Register
#define I2C_DATA_CMD    0x10  // Data Command Register
#define I2C_SS_SCL_HCNT 0x14  // Standard Speed Clock SCL High Count
#define I2C_SS_SCL_LCNT 0x18  // Standard Speed Clock SCL Low Count
#define I2C_FS_SCL_HCNT 0x1C  // Fast Speed Clock SCL High Count
#define I2C_FS_SCL_LCNT 0x20  // Fast Speed Clock SCL Low Count
#define I2C_INTR_STAT   0x2C  // Interrupt Status Register
#define I2C_INTR_MASK   0x30  // Interrupt Mask Register
#define I2C_RAW_INTR_STAT 0x34 // Raw Interrupt Status Register
#define I2C_RX_TL       0x38  // Receive FIFO Threshold Register
#define I2C_TX_TL       0x3C  // Transmit FIFO Threshold Register
#define I2C_CLR_INTR    0x40  // Clear Combined and Individual Interrupt Register
#define I2C_CLR_RX_UNDER 0x44 // Clear RX_UNDER Interrupt Register
#define I2C_CLR_RX_OVER 0x48  // Clear RX_OVER Interrupt Register
#define I2C_CLR_TX_OVER 0x4C  // Clear TX_OVER Interrupt Register
#define I2C_CLR_RD_REQ  0x50  // Clear RD_REQ Interrupt Register
#define I2C_CLR_TX_ABRT 0x54  // Clear TX_ABRT Interrupt Register
#define I2C_CLR_RX_DONE 0x58  // Clear RX_DONE Interrupt Register
#define I2C_CLR_ACTIVITY 0x5C // Clear ACTIVITY Interrupt Register
#define I2C_CLR_STOP_DET 0x60 // Clear STOP_DET Interrupt Register
#define I2C_CLR_START_DET 0x64 // Clear START_DET Interrupt Register
#define I2C_CLR_GEN_CALL 0x68 // Clear GEN_CALL Interrupt Register
#define I2C_ENABLE      0x6C  // Enable Register
#define I2C_STATUS      0x70  // Status Register
#define I2C_TXFLR       0x74  // Transmit FIFO Level Register
#define I2C_RXFLR       0x78  // Receive FIFO Level Register
#define I2C_TX_ABRT_SOURCE 0x80 // Transmit Abort Source Register

// Strutture per la comunicazione I2C
typedef struct {
    uint16 address;
    uint8* buffer;
    size_t length;
    uint32 flags;
} i2c_msg;

#define I2C_M_RD 0x0001 // Flag per operazione di lettura

struct i2c_device {
    uint32 base_addr;
    pci_info pci;
    uint32 index;
    area_id register_area;
    uint8* registers;
    sem_id sem;
};

static i2c_device* gDeviceList = NULL;
static uint32 gDeviceCount = 0;

static pci_module_info* gPCIModule;

#define DEBUG_LEVEL 3

// Prototipi delle funzioni
static status_t i2c_open(const char* name, uint32 flags, void** cookie);
static status_t i2c_close(void* cookie);
static status_t i2c_free(void* cookie);
static status_t i2c_control(void* cookie, uint32 op, void* arg, size_t len);
static status_t i2c_read(void* cookie, off_t position, void* buffer, size_t* numBytes);
static status_t i2c_write(void* cookie, off_t position, const void* buffer, size_t* numBytes);

static void debug_log(int level, const char* format, ...) {
    if (level <= DEBUG_LEVEL) {
        char buffer[512];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        dprintf(DRIVER_NAME ": %s\n", buffer);
    }
}

static inline uint32 read32(i2c_device* device, uint32 reg) {
    return *(volatile uint32*)(device->registers + reg);
}

static inline void write32(i2c_device* device, uint32 reg, uint32 value) {
    *(volatile uint32*)(device->registers + reg) = value;
}

static status_t wait_for_bus_free(i2c_device* device) {
    int timeout = 1000; // 1 secondo di timeout
    while (timeout > 0) {
        if (!(read32(device, I2C_STATUS) & 0x1)) { // Se il bit ACTIVITY è 0, il bus è libero
            return B_OK;
        }
        snooze(1000); // Attendi 1 ms
        timeout--;
    }
    return B_TIMED_OUT;
}

static status_t i2c_transfer(i2c_device* device, i2c_msg* msgs, int num) {
    status_t status;
    
    for (int i = 0; i < num; i++) {
        write32(device, I2C_TAR, msgs[i].address);
        
        status = wait_for_bus_free(device);
        if (status != B_OK) {
            debug_log(1, "Timeout attendendo che il bus I2C sia libero");
            return status;
        }
        
        if (msgs[i].flags & I2C_M_RD) {
            for (size_t j = 0; j < msgs[i].length; j++) {
                write32(device, I2C_DATA_CMD, 0x100); // 0x100 indica un'operazione di lettura
                
                while (!(read32(device, I2C_STATUS) & 0x08)) {
                    snooze(10);
                }
                
                msgs[i].buffer[j] = read32(device, I2C_DATA_CMD) & 0xFF;
            }
        } else {
            for (size_t j = 0; j < msgs[i].length; j++) {
                while (!(read32(device, I2C_STATUS) & 0x02)) {
                    snooze(10);
                }
                
                write32(device, I2C_DATA_CMD, msgs[i].buffer[j]);
            }
        }
    }
    
    return B_OK;
}

status_t init_hardware(void) {
    debug_log(1, "init_hardware() chiamata");
    return B_OK;
}

static status_t probe_i2c_devices() {
    debug_log(1, "Iniziando la ricerca dei dispositivi I2C Tiger Lake");
    pci_info info;
    uint32 index = 0;

    while (gPCIModule->get_nth_pci_info(index++, &info) == B_OK) {
        if (info.vendor_id == INTEL_VENDOR_ID && info.device_id == INTEL_TIGERLAKE_I2C_DEVICE_ID) {
            debug_log(2, "Trovato controller I2C Tiger Lake al bus %02x, device %02x, funzione %x",
                      info.bus, info.device, info.function);
            
            i2c_device* new_devices = (i2c_device*)realloc(gDeviceList, (gDeviceCount + 1) * sizeof(i2c_device));
            if (new_devices == NULL) {
                debug_log(1, "Errore: impossibile allocare memoria per il nuovo dispositivo");
                return B_NO_MEMORY;
            }
            
            gDeviceList = new_devices;
            gDeviceList[gDeviceCount].base_addr = info.u.h0.base_registers[0];
            memcpy(&gDeviceList[gDeviceCount].pci, &info, sizeof(pci_info));
            gDeviceList[gDeviceCount].index = gDeviceCount;

            gDeviceList[gDeviceCount].register_area = map_physical_memory("i2c_tigerlake_regs",
                (phys_addr_t)gDeviceList[gDeviceCount].base_addr, B_PAGE_SIZE,
                B_ANY_KERNEL_ADDRESS, B_KERNEL_READ_AREA | B_KERNEL_WRITE_AREA,
                (void**)&gDeviceList[gDeviceCount].registers);

            if (gDeviceList[gDeviceCount].register_area < B_OK) {
                debug_log(1, "Errore: impossibile mappare i registri del dispositivo");
                return gDeviceList[gDeviceCount].register_area;
            }

            gDeviceCount++;
            
            debug_log(2, "Controller I2C aggiunto alla lista dei dispositivi");
        }
    }

    debug_log(1, "Ricerca completata. Trovati %d controller I2C Tiger Lake", gDeviceCount);

    if (gDeviceCount == 0) {
        debug_log(1, "Nessun controller I2C Tiger Lake trovato");
        return B_ERROR;
    }

    return B_OK;
}

status_t init_driver(void) {
    debug_log(1, "init_driver() chiamata");
    
    if (get_module(B_PCI_MODULE_NAME, (module_info**)&gPCIModule) != B_OK) {
        debug_log(1, "Errore: impossibile ottenere il modulo PCI");
        return B_ERROR;
    }
    debug_log(2, "Modulo PCI ottenuto con successo");

    status_t status = probe_i2c_devices();
    if (status != B_OK) {
        debug_log(1, "probe_i2c_devices() fallita con status: %d", status);
        put_module(B_PCI_MODULE_NAME);
        return status;
    }
    debug_log(2, "probe_i2c_devices() completata con successo");

    debug_log(1, "init_driver() completata con successo. Trovati %d dispositivi", gDeviceCount);
    return B_OK;
}

void uninit_driver(void) {
    debug_log(1, "uninit_driver() chiamata");
    for (uint32 i = 0; i < gDeviceCount; i++) {
        if (gDeviceList[i].register_area >= B_OK) {
            delete_area(gDeviceList[i].register_area);
        }
    }
    free(gDeviceList);
    put_module(B_PCI_MODULE_NAME);
}

const char** publish_devices(void) {
    debug_log(1, "publish_devices() chiamata");
    if (gDeviceCount == 0) {
        debug_log(1, "Nessun dispositivo da pubblicare");
        return NULL;
    }

    const char** devices = (const char**)calloc(gDeviceCount + 1, sizeof(char*));
    if (devices == NULL) {
        debug_log(1, "Errore: impossibile allocare memoria per i nomi dei dispositivi");
        return NULL;
    }

    for (uint32 i = 0; i < gDeviceCount; i++) {
        char* name = (char*)malloc(64);
        if (name == NULL) {
            debug_log(1, "Errore: impossibile allocare memoria per il nome del dispositivo %d", i);
            for (uint32 j = 0; j < i; j++) {
                free((void*)devices[j]);
            }
            free(devices);
            return NULL;
        }
        snprintf(name, 64, DEVICE_NAME, i);
        devices[i] = name;
        debug_log(2, "Pubblicato dispositivo: %s", name);
    }
    devices[gDeviceCount] = NULL;

    return devices;
}

device_hooks* find_device(const char* name) {
    debug_log(2, "find_device() chiamata per il dispositivo: %s", name);
    static device_hooks hooks = {
        i2c_open,
        i2c_close,
        i2c_free,
        i2c_control,
        i2c_read,
        i2c_write
    };

    for (uint32 i = 0; i < gDeviceCount; i++) {
        char device_name[64];
        snprintf(device_name, sizeof(device_name), DEVICE_NAME, i);
        if (strcmp(name, device_name) == 0) {
            return &hooks;
        }
    }
    return NULL;
}

static status_t i2c_open(const char* name, uint32 flags, void** cookie) {
    debug_log(2, "i2c_open() chiamata per il dispositivo: %s", name);
    int index;
    if (sscanf(name, DEVICE_NAME, &index) != 1) {
        debug_log(1, "Errore: nome del dispositivo non valido");
        return B_ERROR;
    }
    if (index < 0 || index >= (int)gDeviceCount) {
        debug_log(1, "Errore: indice del dispositivo non valido");
        return B_ERROR;
    }
    *cookie = (void*)&gDeviceList[index];
    i2c_device* device = (i2c_device*)*cookie;
    
    write32(device, I2C_ENABLE, 0); // Disabilita il controller
    
    write32(device, I2C_FS_SCL_HCNT, 50); // Valori da calcolare in base al clock del bus
    write32(device, I2C_FS_SCL_LCNT, 50);
    
    write32(device, I2C_INTR_MASK, 0); // Disabilita tutte le interruzioni per ora
    
    write32(device, I2C_ENABLE, 1); // Abilita il controller
    
    debug_log(2, "Controller I2C inizializzato");
    
    device->sem = create_sem(1, "i2c_lock");
    if (device->sem < B_OK) {
        debug_log(1, "Errore: impossibile creare il semaforo");
        return device->sem;
    }
    
    return B_OK;
}

static status_t i2c_close(void* cookie) {
    i2c_device* device = (i2c_device*)cookie;
    debug_log(2, "i2c_close() chiamata");
    delete_sem(device->sem);
    return B_OK;
}

static status_t i2c_free(void* cookie) {
    debug_log(2, "i2c_free() chiamata");
    return B_OK;
}

static status_t i2c_read(void* cookie, off_t position, void* buffer, size_t* numBytes) {
    i2c_device* device = (i2c_device*)cookie;
    debug_log(2, "i2c_read() chiamata, posizione: %lld, bytes richiesti: %zu", position, *numBytes);
    
    i2c_msg msg;
    msg.address = position & 0xFFFF; // Usa la posizione come indirizzo del dispositivo
    msg.buffer = (uint8*)buffer;
    msg.length = *numBytes;
    msg.flags = I2C_M_RD;
    
    status_t status = acquire_sem(device->sem);
    if (status != B_OK) {
        debug_log(1, "Errore: impossibile acquisire il semaforo");
        return status;
    }
    
    status = i2c_transfer(device, &msg, 1);
    
    release_sem(device->sem);
    
    if (status == B_OK) {
        debug_log(2, "Letti %zu bytes", *numBytes);
    } else {
        debug_log(1, "Errore durante la lettura I2C: %s", strerror(status));
        *numBytes = 0;
    }
    
    return status;
}

static status_t i2c_write(void* cookie, off_t position, const void* buffer, size_t* numBytes) {
    i2c_device* device = (i2c_device*)cookie;
    debug_log(2, "i2c_write() chiamata, posizione: %lld, bytes da scrivere: %zu", position, *numBytes);
    
    i2c_msg msg;
    msg.address = position & 0xFFFF; // Usa la posizione come indirizzo del dispositivo
    msg.buffer = (uint8*)buffer;
    msg.length = *numBytes;
    msg.flags = 0; // Scrittura
    
    status_t status = acquire_sem(device->sem);
    if (status != B_OK) {
        debug_log(1, "Errore: impossibile acquisire il semaforo");
        return status;
    }
    
    status = i2c_transfer(device, &msg, 1);
    
    release_sem(device->sem);
    
    if (status == B_OK) {
        debug_log(2, "Scritti %zu bytes", *numBytes);
    } else {
        debug_log(1, "Errore durante la scrittura I2C: %s", strerror(status));
        *numBytes = 0;
    }
    
    return status;
}

static status_t i2c_control(void* cookie, uint32 op, void* arg, size_t len) {
    i2c_device* device = (i2c_device*)cookie;
    debug_log(2, "i2c_control() chiamata con op: %u, len: %zu", op, len);

    switch (op) {
        case 2000: { // Esempio: Esegui un trasferimento I2C personalizzato
            if (len != sizeof(i2c_msg)) {
                return B_BAD_VALUE;
            }
            i2c_msg* msg = (i2c_msg*)arg;
            return i2c_transfer(device, msg, 1);
        }

        case 2001: { // Esempio: Imposta la velocità del bus I2C
            if (len != sizeof(uint32)) {
                return B_BAD_VALUE;
            }
            uint32 speed = *(uint32*)arg;
            // Implementa la logica per impostare la velocità del bus
            // Questo richiederà calcoli basati sul clock del sistema
            debug_log(2, "Impostazione della velocità del bus I2C a %u Hz", speed);
            return B_OK;
        }

        // Aggiungi altri casi per diverse operazioni di controllo

        default:
            debug_log(1, "Operazione di controllo non supportata: %u", op);
            return B_BAD_VALUE;
    }
}

// Esporta le funzioni del driver
extern "C" {
    _EXPORT status_t init_hardware(void);
    _EXPORT status_t init_driver(void);
    _EXPORT void uninit_driver(void);
    _EXPORT const char** publish_devices(void);
    _EXPORT device_hooks* find_device(const char* name);
}
