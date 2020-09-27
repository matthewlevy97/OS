#pragma once

#include <stdint.h>

typedef struct {
	uint32_t counter;
} atomic_t;

void     atomic_set(atomic_t*, uint32_t);
uint32_t atomic_get(atomic_t*);

void atomic_inc(atomic_t*);
void atomic_dec(atomic_t*);
char atomic_dec_and_test(atomic_t*);