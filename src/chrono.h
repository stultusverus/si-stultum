#pragma once

#include <stdint.h>

extern volatile uint64_t ms_count;

void init_PIT(uint32_t frequency);