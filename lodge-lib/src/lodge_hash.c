#include "lodge_hash.h"

#include "murmur3.h"

#define LODGE_HASH_SEED 0

uint32_t lodge_hash_murmur3_32(const void *data, size_t data_len)
{
	uint32_t hash;
	MurmurHash3_x86_32(data, (int)data_len, LODGE_HASH_SEED, &hash);
	return hash;
}