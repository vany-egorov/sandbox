#include "h264.h"


static inline uint32_t get_bit(const uint8_t * const base, uint32_t offset) {
	return ((*(base + (offset >> 0x3))) >> (0x7 - (offset & 0x7))) & 0x1;
}

inline uint32_t h264_bitstream_get_bits(const uint8_t * const base, uint32_t * const offset, uint8_t bits) {
	int i = 0;
	uint32_t value = 0;
	for (i = 0; i < bits; i++) {
		value = (value << 1) | (get_bit(base, (*offset)++) ? 1 : 0);
	}
	return value;
}

// This function implement decoding of exp-Golomb codes of zero range (used in H.264).
uint32_t h264_bitstream_decode_u_golomb(const uint8_t * const base, uint32_t * const offset) {
	uint32_t zeros = 0;

	// calculate zero bits. Will be optimized.
	while (0 == get_bit(base, (*offset)++)) zeros++;

	// insert first 1 bit
	uint32_t info = 1 << zeros;

	int32_t i = 0;
	for (i = zeros - 1; i >= 0; i--) {
		info |= get_bit(base, (*offset)++) << i;
	}

	return (info - 1);
}
