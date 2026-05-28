
#include <inttypes.h>

void printBitSequence(uint8_t value);

void setBitsToOne(uint8_t* pointerToValue , uint8_t bitmask);

void setBitsToZero(uint8_t* pointerToValue, uint8_t bitmask);

void flipBits(uint8_t* pointerToValue, uint8_t bitmask);

void shiftBitsToTheRight(uint8_t* pointerToValue,uint8_t amount);

void shiftBitsToTheLeft(uint8_t* pointerToValue, uint8_t amount);

uint16_t combineBytes(uint8_t upperByte, uint8_t lowerByte);