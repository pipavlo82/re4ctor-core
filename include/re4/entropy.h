#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Заповнює dst n байтами ентропії.
// Повертає 0 при успіху, -1 при помилці.
int re4_sys_entropy(uint8_t *dst, size_t n);

#ifdef __cplusplus
}
#endif
