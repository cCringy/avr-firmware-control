#include <bit_manipulation.h>

void printBitSequence(uint8_t value){
	for(int i = 7 ; i >= 0; i--){
		printf("%c", (value & (1 << i)) ? '1' : '0');
	}
}

void setBitsToOne(uint8_t* pointerToValue, uint8_t bitmask){
	*pointerToValue |= bitmask;
}

void setBitsToZero(uint8_t* pointerToValue, uint8_t bitmask){
	*pointerToValue &= ~bitmask;
}

void flipBits(uint8_t* pointerToValue,uint8_t bitmask){
	*pointerToValue ^= bitmask;
}

void shiftBitsToTheRight(uint8_t* pointerToValue,uint8_t amount){
	*pointerToValue >>= amount;
}

void shiftBitsToTheLeft(uint8_t* pointerToValue, uint8_t amount){
	*pointerToValue <<= amount;
}

uint16_t combineBytes(uint8_t upperByte, uint8_t lowerByte){
	uint16_t result = (upperByte<<8)|lowerByte;
	return result;
}