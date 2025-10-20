#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// HW entropy (x86_64): повертає 1 якщо успішно, 0 якщо ні
int re4_hw_rdrand(uint64_t *out);
int re4_hw_rdseed(uint64_t *out);

// Системна ентропія (/dev/urandom або інше джерело), повертає кількість байтів
int re4_sys_entropy(uint8_t *dst, size_t n);

#ifdef __cplusplus
}
#endif
