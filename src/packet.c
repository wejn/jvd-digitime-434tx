#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <math.h>

#define PACKET_SIZE 32

// adapted from
// https://github.com/merbanan/rtl_433/blob/master/src/util.c
uint8_t crc4(const bool *message, int nbits, uint8_t polynomial, uint8_t init)
{
    unsigned remainder = init << 4; // LSBs are unused
    unsigned poly = polynomial << 4;
    unsigned bit;

    for (; nbits > 0; nbits -= 8, message += 8) {
		uint8_t nowbits = (nbits > 8 ? 8 : nbits);
		for (uint8_t i = 0; i < nowbits; i++) {
			remainder ^= (message[i] << (8-1-i));
		}
        for (bit = 0; bit < nowbits; bit++) {
            if (remainder & 0x80) {
                remainder = (remainder << 1) ^ poly;
            } else {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder >> 4 & 0x0f; // discard the LSBs
}

void init_with_temp(bool *pattern, float temp, bool battery_low, int channel) {
  bool pattern_template[PACKET_SIZE] = {
    0,0,0,1, // [0..3] static pattern
    0,0,1,0, // [4..7] channel (0=2, 2=1, 4=3)
    1,0,1,1,1,1,1,1,1,1, // [8..17] temperature_c = pattern - 500 / 10
    0,0,0,0,0,0,0, // [18..24] static pattern
    0, // [25] battery (0 = OK); WARN: our receiver takes bat low (1) as sticky
    0, // [26] static pattern
    0, // [27] unknown
    0,0,0,0 // [28..31] crc-4 of [0..27], poly 0x9, init 0x1
  };

  for (uint8_t i = 0; i < PACKET_SIZE; i++) {
    pattern[i] = pattern_template[i];
  }

  // battery
  pattern[25] = battery_low ? 1 : 0;

  // channel
  uint8_t ch_map[] = {1 /* suspect */, 2, 0, 4};
  uint8_t ch = (channel >= 1 && channel <= 3) ? ch_map[channel] : 0;
  for (uint8_t i = 0; i < 4; i++) {
    pattern[4 + i] = ch & 1<<(4-1-i) ? 1 : 0;
  }

  // temp
  uint16_t t = round(temp * 10.0) + 500;
  for (uint8_t i = 0; i < 10; i++) {
    pattern[8 + i] = t & 1<<(10-1-i) ? 1 : 0;
  }

  // crc
  uint8_t crc = crc4(pattern, PACKET_SIZE-4, 0x9, 0x1);
  for (uint8_t i = 0; i < 4; i++) {
    pattern[28 + i] = (crc & 1<<(4-1-i)) ? 1 : 0;
  }
}

#ifdef _PKTGEN_MAIN_
int main(void) {
	bool pattern[PACKET_SIZE] = {
	  0,0,0,1, // static pattern
	  0,0,1,0, // channel (0=2, 2=1, 4=3)
	  1,0,1,1,1,1,1,1,1,1, // temperature_c = pattern - 500 / 10
	  0,0,0,0,0,0,0, // static pattern
	  0, // battery (0 = OK)
	  0, // static pattern
	  0, // unknown
	  1,0,1,0 // crc-4, poly 0x9, init 0x1
	};

	bool test_pattern[][32] = {
		{0,0,0,1,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0},
		{0,0,0,1,0,0,1,0,1,0,1,1,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
		{0,0,0,1,0,0,1,0,1,0,1,1,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
		{0,0,0,1,0,0,1,0,1,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,0,1,0},
		{0,0,0,1,0,0,1,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
		{0,0,0,1,0,0,1,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,0},
		{0,0,0,1,0,0,1,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{0,0,0,1,0,0,1,0,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0},
		{0,0,0,1,0,0,1,0,1,1,0,0,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0},
		{0,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0},
		{0,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
		{0,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
	};


	bool packet[32];

	init_with_temp(packet, 26.7, 0, 1);

	for (uint16_t i = 0; i < PACKET_SIZE; i++) {
		printf("%d", packet[i] ? 1 : 0);
	}
	printf("\n");

	for (uint16_t i = 0; i < PACKET_SIZE; i++) {
		printf("%d", pattern[i] ? 1 : 0);
	}
	printf("\n");

	for (uint16_t i = 0; i < sizeof(test_pattern) / sizeof(test_pattern[0]); i++) {
		uint8_t c = crc4(test_pattern[i], PACKET_SIZE-4, 0x9, 0x1);
		bool *t = test_pattern[i];
		uint8_t exp = t[28] << 3 | t[29] << 2 | t[30] << 1 | t[31];
		if (exp != c) {
			printf("test fail for: %d: want=%d, got=%d\n", i, exp, c);
		}
		if (crc4(test_pattern[i], PACKET_SIZE, 0x9, 0x1) != 0) {
			printf("pattern fail for: %d\n", i);
		}
	}
}
#endif
