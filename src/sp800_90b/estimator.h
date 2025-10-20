#pragma once
#include <stddef.h>
/* Very simple SP800-90B Most-Common-Value estimator over bytes.
 * Returns estimated min-entropy per byte (0..8). */
double re4_90b_mcv_min_entropy(const unsigned char *buf, size_t n);
