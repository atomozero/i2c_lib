#include "i2c.h"
#include <OS.h>
#include <stdio.h>
#include <string.h>

int main() {
    I2CBus bus(0);  // Assumiamo che il bus I2C sia il numero 0

    status_t status = bus.init();
    if (status != B_OK) {
        printf("Errore nell'inizializzazione del bus I2C: %s\n", strerror(static_cast<int>(status)));
        return 1;
    }

    // Test di scrittura
    uint8 address = 0x50;  // Indirizzo del dispositivo I2C (esempio)
    uint8 data[] = {0x01, 0x02, 0x03};
    status = bus.write(address, data, sizeof(data));
    if (status == B_OK) {
        printf("Scrittura I2C riuscita\n");
    } else {
        printf("Errore nella scrittura I2C: %s\n", strerror(static_cast<int>(status)));
    }

    // Test di lettura
    uint8 buffer[3];
    status = bus.read(address, buffer, sizeof(buffer));
    if (status == B_OK) {
        printf("Lettura I2C riuscita: %02X %02X %02X\n", buffer[0], buffer[1], buffer[2]);
    } else {
        printf("Errore nella lettura I2C: %s\n", strerror(static_cast<int>(status)));
    }

    return 0;
}