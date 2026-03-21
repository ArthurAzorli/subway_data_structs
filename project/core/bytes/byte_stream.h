#ifndef BYTE_STREAM_H
#define BYTE_STREAM_H

#include "byte_array.h"
#include "../utils/types.h"

#define  UINT8_BYTES_COUNT 1
#define  UINT16_BYTES_COUNT 2
#define  UINT24_BYTES_COUNT 3
#define  UINT32_BYTES_COUNT 4
#define  UINT64_BYTES_COUNT 8

void ByteStream_addIntegerBack(struct ByteArray *bytes, const uint64_t value, const size_t bytesCount);

void ByteStream_addIntegerFront(struct ByteArray *bytes, const uint64_t value, const size_t bytesCount);

void ByteStream_setInteger(struct ByteArray *bytes, const size_t index, const uint64_t value, const size_t bytesCount);

bool ByteStream_readInteger(struct ByteArray *bytes, const size_t index, const size_t bytesCount, uint64_t *result);

#endif //BYTE_STREAM_H
