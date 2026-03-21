#include "byte_stream.h"

void ByteStream_addIntegerBack(struct ByteArray *bytes, const uint64_t value, const size_t bytesCount) {
    if (bytes == NULL) return;
    for (size_t i = 1; i <= bytesCount; i++) {
        const uint8_t byte = value >> (8 * (bytesCount - i)) & 0xFF;
        ByteArray_pushBack(bytes, byte);
    }
}

void ByteStream_addIntegerFront(struct ByteArray *bytes, const uint64_t value, const size_t bytesCount) {
    if (bytes == NULL) return;
    for (size_t i = 0; i < bytesCount; i++) {
        const uint8_t byte = value >> (8 * i) & 0xFF;
        ByteArray_pushFront(bytes, byte);
    }
}

void ByteStream_setInteger(struct ByteArray *bytes, const size_t index, const uint64_t value, const size_t bytesCount) {
    if (bytes == NULL) return;
    const size_t length = ByteArray_length(bytes);
    if (index >= length) return;

    for (size_t i = 0; i < bytesCount; i++) {
        const uint8_t byte = value >> (8 * (bytesCount - i - 1)) & 0xFF;
        if (index + i < length) {
            ByteArray_set(bytes, index + i, byte);
        } else {
            ByteArray_pushBack(bytes, byte);
        }
    }
}

bool ByteStream_readInteger(struct ByteArray *bytes, const size_t index, const size_t bytesCount, uint64_t *result) {
    if (bytes == NULL) return false;
    const size_t length = ByteArray_length(bytes);
    if (index + bytesCount > length) return false;

    *result = 0;
    for (size_t i = 0; i < bytesCount; i++) {
        const uint8_t byte = *ByteArray_get(bytes, index + i);
        *result |= byte << (8 * (bytesCount - i - 1));
    }
    return true;
}
