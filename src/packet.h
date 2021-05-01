#include <inttypes.h>

#define PACKET_SIZE 32

uint8_t crc4(bool const *message, int nbits, uint8_t polynomial, uint8_t init);

void init_with_temp(bool *pattern, float temp, bool battery_low, int channel);
