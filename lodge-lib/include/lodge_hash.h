#ifndef _LODGE_HASH_H
#define _LODGE_HASH_H

#include <stdint.h>

uint32_t lodge_hash_murmur3_32(const void *data, size_t data_len);

#endif